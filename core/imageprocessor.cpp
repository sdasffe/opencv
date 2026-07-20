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
 *   因为此时 ROI 可能刚被用户拖过，必须以 Widget 最新的 getCurrentRoiInfo()
 *   为准。Widget 收到 requestReprocess 后会：
 *     setRoi(getCurrentRoiInfo()); reprocess();
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
 * @brief 更新本次重算使用的 ROI
 *
 * 必须在 reprocess() 之前由 Widget 调用。
 * roi.shape == None 时，各 Block 内 RoiProcess::apply 会对全图生效。
 */
void ImageProcessor::setRoi(const RoiInfo &roi)
{
    m_roi = roi;
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
 *           current = block.process(current, m_roi)
 *   m_result = current
 *   通知 UI：processingFinished(耗时ms)
 *
 * -------------------------------------------------------------------------
 * 要点
 * -------------------------------------------------------------------------
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

    // ---------- 流水线 ----------
    QPixmap current = m_original;
    for (BaseBlock *block : m_blocks) {
        if (!block->isEnabled())
            continue;  // 跳过关闭的块

        // process 是纯虚函数，实际跑到 BinarizationBlock::process 等
        // 传入同一份 m_roi：整条链共用当前 ROI（由 Widget 同步）
        current = block->process(current, m_roi);
    }

    m_result = current;
    m_lastElapsed = tm.elapsedMs();

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
    if (hasImage())
        emit requestReprocess();
}

/**
 * @brief 槽：某个块的启用开关变了
 *
 * enabled 参数本身不用：无论打开还是关闭，都需要整链重算一次，
 * 才能让画面与当前“启用集合”一致。
 */
void ImageProcessor::onBlockEnabledChanged(bool /*enabled*/)
{
    if (hasImage())
        emit requestReprocess();
}
