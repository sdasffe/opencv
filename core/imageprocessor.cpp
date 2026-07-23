/**
 * @file imageprocessor.cpp
 * @brief 处理链调度引擎实现（不含 UI，只负责“按顺序跑算法”）
 *
 * Widget 调用 setOriginalImage / addBlock / setRois / reprocess；
 * 本类循环 block->process(current, m_rois)，结果经 processingFinished 回传 UI。
 */

#include "imageprocessor.h"
#include "../utils/applogger.h"

#include <QtGlobal>
#include <QStringList>

/**
 * @brief 构造处理引擎；成员靠类内默认值（空 pixmap、空列表、耗时 0）
 */
ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{
    // 无额外初始化；m_original/m_result 为空，m_blocks/m_rois 为空列表
}

/**
 * @brief 设置用户加载的原始图像，并重置结果为原图
 *
 * 调用时机：Widget::loadImageFromPath 成功读图后。
 * 不触发重算；若右侧已有块，由 Widget 末尾调 onApplyProcessing()。
 */
void ImageProcessor::setOriginalImage(const QPixmap &pixmap)
{
    m_original = pixmap;                                             // 保存只读数据源
    m_result = pixmap;                                               // 未跑链时“结果”应等于原图
}

/**
 * @brief 把结果图同步回原图（不跑处理链、不修改 m_original）
 *
 * 调用时机：Widget 清空处理链后，避免对比/保存仍显示旧 m_result。
 */
void ImageProcessor::resetResultToOriginal()
{
    m_result = m_original;                                           // 仅重置输出；原图指针数据不变
}

/**
 * @brief 把处理块接到链尾，并监听 paramsChanged / enabledChanged
 *
 * 发 requestReprocess 而非直接 reprocess：ROI 须由 Widget 最新 getAllRoiInfo() 同步。
 * 调用时机：Widget::addBlockToPanel。
 */
void ImageProcessor::addBlock(BaseBlock *block)
{
    if (!block) return;                                              // 空指针防御

    m_blocks.append(block);                                          // 顺序=执行顺序=面板从上到下

    connect(block, &BaseBlock::paramsChanged,                        // 阈值/核大小等参数变化
            this, &ImageProcessor::onBlockParamsChanged);
    connect(block, &BaseBlock::enabledChanged,                       // 标题栏启用开关变化
            this, &ImageProcessor::onBlockEnabledChanged);

    if (hasImage())
        emit requestReprocess();                                     // 有图则请求 Widget 统一重算
}

/**
 * @brief 从处理链移除块（不负责释放内存；由 Widget deleteLater）
 *
 * 调用时机：用户点块 ✕ 或 onClearBlocks。
 */
void ImageProcessor::removeBlock(BaseBlock *block)
{
    if (!block) return;                                              // 空指针防御

    disconnect(block, nullptr, this, nullptr);                       // 断开 block→this 全部信号，防野信号
    m_blocks.removeOne(block);                                       // 从链中去掉（不 delete）

    if (hasImage())
        emit requestReprocess();                                     // 剩余块对原图再跑一遍
}

/**
 * @brief 调整块在处理链中的下标（顺序即执行顺序）
 *
 * 调用时机：用户在右侧面板拖动块标题换序。
 */
void ImageProcessor::moveBlock(BaseBlock *block, int toIndex)
{
    if (!block) return;                                              // 空指针防御

    const int from = m_blocks.indexOf(block);                        // 当前下标
    if (from < 0) return;                                            // 不在链中则忽略

    toIndex = qBound(0, toIndex, m_blocks.size() - 1);               // 夹紧到 [0, size-1]
    if (from == toIndex) return;                                     // 位置未变则无需重算

    m_blocks.move(from, toIndex);                                    // QList 移动，顺序与面板一致

    if (hasImage())
        emit requestReprocess();                                     // 顺序变化需整链重算
}

/**
 * @brief 更新本次 reprocess 使用的 ROI 列表（只写入缓存，不触发重算）
 *
 * 必须在 reprocess() 之前由 Widget 调用；空列表时各 Block 对全图生效。
 */
void ImageProcessor::setRois(const QList<RoiInfo> &rois)
{
    m_rois = rois;                                                   // 拷贝 ROI 纯数据快照
}

/**
 * @brief 按处理链顺序执行所有启用的块，写入 m_result 并通知 UI
 *
 * 空链或全部块禁用：m_result = m_original（原图直通）。
 * 每一块的输入是上一块的输出，块顺序很重要。
 *
 * @return 本次墙钟耗时（毫秒）；无图返回 0
 */
qint64 ImageProcessor::reprocess()
{
    if (!hasImage()) return 0;                                       // 无图无法处理

    TimeMeasurer tm("处理链", false);                                // false=析构时不自动 qDebug

    QPixmap current = m_original;                                    // 流水线当前图；空链时保持原图
    QStringList ran;                                                 // 已执行块名+耗时，供日志串联
    for (BaseBlock *block : m_blocks) {
        if (!block->isEnabled()) {                                   // 禁用块跳过，相当于临时移出链
            AppLogger::info(QStringLiteral("跳过处理块"),
                            QStringLiteral("name=%1 (已禁用)").arg(block->blockName()));
            continue;
        }

        TimeMeasurer step(block->blockName(), false);                // 单块计时
        current = block->process(current, m_rois);                   // 上一块输出 + ROI → 本块输出
        const qint64 stepMs = step.elapsedMs();                      // 本块墙钟毫秒
        ran << QStringLiteral("%1(%2ms)").arg(block->blockName()).arg(stepMs); // 记入步骤摘要
        AppLogger::info(QStringLiteral("执行处理块"),
                        QStringLiteral("name=%1 elapsed=%2 ms rois=%3")
                            .arg(block->blockName())
                            .arg(stepMs)
                            .arg(m_rois.size()));
    }

    m_result = current;                                              // 最后一级输出即为结果图
    m_lastElapsed = tm.elapsedMs();                                  // 整链总耗时

    AppLogger::info(QStringLiteral("流水线结束"),
                    QStringLiteral("total=%1 ms steps=[%2]")
                        .arg(m_lastElapsed)
                        .arg(ran.join(QStringLiteral(" -> "))));

    emit processingFinished(m_lastElapsed);                          // Widget::onProcessingFinished 刷新画布
    return m_lastElapsed;
}

/**
 * @brief 槽：某块参数变化 → 有图时 emit requestReprocess
 *
 * 触发路径：SpinBox 等 → block::paramsChanged → 本槽 → Widget::onApplyProcessing
 */
void ImageProcessor::onBlockParamsChanged()
{
    if (hasImage()) {
        auto *block = qobject_cast<BaseBlock *>(sender());           // 定位信号来源块
        AppLogger::info(QStringLiteral("块参数变化"),
                        block ? block->blockName() : QStringLiteral("(unknown)"));
        emit requestReprocess();                                     // 由 Widget 先同步 ROI 再 reprocess
    }
}

/**
 * @brief 槽：某块启用开关变化 → 有图时 emit requestReprocess
 *
 * enabled 参数仅用于日志；开/关都需整链重算以匹配当前启用集合。
 */
void ImageProcessor::onBlockEnabledChanged(bool enabled)
{
    if (hasImage()) {
        auto *block = qobject_cast<BaseBlock *>(sender());           // 定位信号来源块
        AppLogger::info(QStringLiteral("块启用变化"),
                        QStringLiteral("name=%1 enabled=%2")
                            .arg(block ? block->blockName() : QStringLiteral("(unknown)"))
                            .arg(enabled ? QStringLiteral("是") : QStringLiteral("否")));
        emit requestReprocess();                                     // 由 Widget 先同步 ROI 再 reprocess
    }
}
