/**
 * @file imageprocessor.cpp
 * @brief 处理链调度引擎实现（不含 UI，只负责“按顺序跑算法”）
 *
 * =============================================================================
 * 本类在整条链路中的位置
 * =============================================================================
 *
 *   Widget（UI）
 *     │  setOriginalImage / addBlock / setRoi / reprocess
 *     ▼
 *   ImageProcessor（本文件）
 *     │  for each enabled block: current = block->process(current, roi)
 *     ▼
 *   BaseBlock 子类（二值化/滤波/...）
 *     │  QPixmap → cv::Mat → 算法 → QPixmap
 *     ▼
 *   processingFinished(耗时) → Widget 刷新画布
 *
 * =============================================================================
 * 设计原则（读代码时记住这三点）
 * =============================================================================
 *   1. Processor 不持有 QGraphicsItem，只持有 RoiInfo 纯数据
 *      → ROI 怎么从场景读出来，是 Widget 的事
 *   2. Processor 不主动在 ROI 拖动时重算
 *      → 块参数变化只发 requestReprocess，由 Widget 统一 setRoi + reprocess
 *   3. 处理链是“纯函数流水线”：原图进，结果出；中间不改 m_original
 */

#include "imageprocessor.h"
#include "../utils/applogger.h"

#include <QtGlobal>
#include <QStringList>

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{
    // 无额外初始化：成员靠类内默认值（空 pixmap、空列表、roi=None、耗时0）
}

/**
 * @brief 设置用户加载的原始图像
 *
 * 调用时机：Widget::loadImageFromPath 成功读图后。
 *
 * 同时把 m_result 设为原图的原因：
 *   - 还没跑处理链时，“结果”就应该等于原图
 *   - 清空处理链后 Widget 也会再调一次，用来重置结果显示
 *
 * 注意：这里不做重算。若右侧已有块，由 Widget 在 loadImageFromPath
 * 末尾自己调 onApplyProcessing()。
 */
void ImageProcessor::setOriginalImage(const QPixmap &pixmap)
{
    m_original = pixmap;
    m_result = pixmap;
}

void ImageProcessor::resetResultToOriginal()
{
    // 不修改 m_original；仅让 resultImage() 回到原图（清空链后对比/保存用）
    m_result = m_original;
}

/**
 * @brief 把一个处理块接到链尾，并监听它的变化信号
 *
 * 调用时机：Widget::addBlockToPanel。
 *
 * 接线说明：
 *   block->paramsChanged  ──► onBlockParamsChanged ──► requestReprocess
 *   block->enabledChanged ──► onBlockEnabledChanged ──► requestReprocess
 *
 * 为什么发 requestReprocess 而不是直接 reprocess()？
 *   因为此时 ROI 可能刚被用户拖过，必须以 Widget 最新的 getAllRoiInfo()
 *   为准。Widget 收到 requestReprocess 后会：
 *     setRois(getAllRoiInfo()); reprocess();
 *
 * 若已有图：立即请求一次重算，让新块马上生效（用户拖入就能看到效果）。
 */
void ImageProcessor::addBlock(BaseBlock *block)
{
    if (!block) return;

    m_blocks.append(block);  // 顺序 = 执行顺序 = 面板从上到下

    connect(block, &BaseBlock::paramsChanged,
            this, &ImageProcessor::onBlockParamsChanged);
    connect(block, &BaseBlock::enabledChanged,
            this, &ImageProcessor::onBlockEnabledChanged);

    if (hasImage())
        emit requestReprocess();
}

/**
 * @brief 从处理链移除块（不负责释放内存）
 *
 * 调用时机：
 *   - 用户点块上的 ✕（Widget lambda 里）
 *   - 清空整条链 onClearBlocks
 *
 * 步骤：
 *   1. disconnect 所有与本 Processor 的连接，避免野信号
 *   2. 从 m_blocks 去掉
 *   3. 若还有图，请求重算（剩余块对原图再跑一遍）
 *
 * 内存：由 Widget 调用 block->deleteLater()，本函数不 delete。
 */
void ImageProcessor::removeBlock(BaseBlock *block)
{
    if (!block) return;

    // 断开 block 发往 this 的所有信号，防止块析构前还触发槽
    disconnect(block, nullptr, this, nullptr);
    m_blocks.removeOne(block);

    if (hasImage())
        emit requestReprocess();
}

/**
 * @brief 调整块在处理链中的下标
 *
 * 调用时机：用户在右侧面板拖动块标题换序。
 * QList::move 后顺序与面板、m_blockList 保持一致，再请求重算。
 */
void ImageProcessor::moveBlock(BaseBlock *block, int toIndex)
{
    if (!block) return;

    const int from = m_blocks.indexOf(block);
    if (from < 0) return;

    toIndex = qBound(0, toIndex, m_blocks.size() - 1);
    if (from == toIndex) return;

    m_blocks.move(from, toIndex);

    if (hasImage())
        emit requestReprocess();
}

/**
 * @brief 更新本次重算使用的 ROI 列表
 *
 * 必须在 reprocess() 之前由 Widget 调用。
 * 空列表时，各 Block 内 RoiProcess::apply 对全图生效。
 * 本函数只写入 m_rois，不 emit requestReprocess。
 */
void ImageProcessor::setRois(const QList<RoiInfo> &rois)
{
    m_rois = rois;
}

/**
 * @brief 【核心】按处理链顺序执行所有启用的块
 *
 * -------------------------------------------------------------------------
 * 算法伪代码
 * -------------------------------------------------------------------------
 *   current = 原图
 *   for block in m_blocks:
 *       if block 启用:
 *           current = block.process(current, m_rois)
 *   m_result = current
 *   通知 UI：processingFinished(耗时ms)
 *
 * -------------------------------------------------------------------------
 * 要点
 * -------------------------------------------------------------------------
 *   - 空链（m_blocks 为空）或全部块禁用：current 保持原图，m_result = m_original
 *   - 未启用的块（标题栏“开”取消勾选）直接跳过，相当于从链中临时移除
 *   - 每一块的输入是上一块的输出，所以块顺序很重要
 *     例：先模糊再二值化 ≠ 先二值化再模糊
 *   - m_original 永不被块修改；只有 m_result 被覆盖
 *   - 计时用 TimeMeasurer：从进入到算完的墙钟时间（含所有块）
 *
 * @return 本次耗时毫秒；无图时返回 0
 */
qint64 ImageProcessor::reprocess()
{
    if (!hasImage()) return 0;

    // 第二个参数 false = 析构时不自动 qDebug 打印（我们只用 elapsedMs）
    TimeMeasurer tm("处理链", false);

    // ---------- 流水线（空链时 current 仍为 m_original） ----------
    QPixmap current = m_original;
    QStringList ran;
    for (BaseBlock *block : m_blocks) {
        if (!block->isEnabled()) {
            AppLogger::info(QStringLiteral("跳过处理块"),
                            QStringLiteral("name=%1 (已禁用)").arg(block->blockName()));
            continue;
        }

        TimeMeasurer step(block->blockName(), false);
        current = block->process(current, m_rois);
        const qint64 stepMs = step.elapsedMs();
        ran << QStringLiteral("%1(%2ms)").arg(block->blockName()).arg(stepMs);
        AppLogger::info(QStringLiteral("执行处理块"),
                        QStringLiteral("name=%1 elapsed=%2 ms rois=%3")
                            .arg(block->blockName())
                            .arg(stepMs)
                            .arg(m_rois.size()));
    }

    m_result = current;
    m_lastElapsed = tm.elapsedMs();

    AppLogger::info(QStringLiteral("流水线结束"),
                    QStringLiteral("total=%1 ms steps=[%2]")
                        .arg(m_lastElapsed)
                        .arg(ran.join(QStringLiteral(" -> "))));

    // Widget::onProcessingFinished 会 refreshDisplay + 更新耗时标签
    emit processingFinished(m_lastElapsed);
    return m_lastElapsed;
}

/**
 * @brief 槽：某个块的参数变了（阈值、核大小、滤波类型……）
 *
 * 触发路径举例：
 *   用户拧 SpinBox
 *     → BinarizationBlock::onLowerChanged
 *     → emit paramsChanged()
 *     → 本函数
 *     → emit requestReprocess()
 *     → Widget::onApplyProcessing()
 */
void ImageProcessor::onBlockParamsChanged()
{
    if (hasImage()) {
        auto *block = qobject_cast<BaseBlock *>(sender());
        AppLogger::info(QStringLiteral("块参数变化"),
                        block ? block->blockName() : QStringLiteral("(unknown)"));
        emit requestReprocess();
    }
}

/**
 * @brief 槽：某个块的启用开关变了
 *
 * enabled 参数本身不用：无论打开还是关闭，都需要整链重算一次，
 * 才能让画面与当前“启用集合”一致。
 */
void ImageProcessor::onBlockEnabledChanged(bool enabled)
{
    if (hasImage()) {
        auto *block = qobject_cast<BaseBlock *>(sender());
        AppLogger::info(QStringLiteral("块启用变化"),
                        QStringLiteral("name=%1 enabled=%2")
                            .arg(block ? block->blockName() : QStringLiteral("(unknown)"))
                            .arg(enabled ? QStringLiteral("是") : QStringLiteral("否")));
        emit requestReprocess();
    }
}
