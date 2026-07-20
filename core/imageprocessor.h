#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QList>
#include "../blocks/baseblock.h"
#include "../roi/roiinfo.h"
#include "../utils/timemeasurer.h"

/**
 * @brief 图像处理调度引擎（处理链的“大脑”，不含任何界面控件）
 *
 * =============================================================================
 * 一句话职责
 * =============================================================================
 *   保存原图/结果图，按顺序调用一串 BaseBlock::process()，算出结果图并通知 UI。
 *
 * =============================================================================
 * 与 Widget 的分工（非常重要）
 * =============================================================================
 *   Widget 负责：
 *     - 打开图片、显示、ROI 图元、拖拽建块
 *     - 从场景读取 RoiInfo，再调用 setRoi()
 *     - 收到 requestReprocess / 用户点应用 时调用 reprocess()
 *     - 收到 processingFinished 后把 resultImage 画到画布
 *
 *   ImageProcessor 负责：
 *     - 持有 m_original / m_result / m_blocks / m_roi
 *     - 流水线循环调用 block->process()
 *     - 参数变化时只发 requestReprocess（不自己 reprocess，好让 Widget 先同步 ROI）
 *
 * =============================================================================
 * 典型调用序列
 * =============================================================================
 *   setOriginalImage(pixmap)          // 加载图
 *   addBlock(binBlock)                // 拖入块（可能立刻 requestReprocess）
 *   addBlock(filterBlock)
 *   setRoi(roiInfo)                   // Widget 每次重算前调用
 *   reprocess()                       // 跑完整条链
 *   → emit processingFinished(ms)
 *
 * =============================================================================
 * 数据流
 * =============================================================================
 *   m_original
 *     → [启用的] block1.process(current, m_roi)
 *     → [启用的] block2.process(current, m_roi)
 *     → ...
 *     → m_result
 */
class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessor(QObject *parent = nullptr);

    /**
     * @brief 设置原图，并把结果重置为原图
     * @note 不触发重算；若需要立刻处理，由 Widget 接着调 onApplyProcessing
     */
    void setOriginalImage(const QPixmap &pixmap);

    /** @brief 用户加载的原始图像（处理链不会修改它） */
    QPixmap originalImage() const { return m_original; }

    /** @brief 最近一次 reprocess 的输出；未处理过时等于原图 */
    QPixmap resultImage() const { return m_result; }

    /** @brief 是否已加载过非空图片 */
    bool hasImage() const { return !m_original.isNull(); }

    /**
     * @brief 把处理块追加到链尾，并监听 paramsChanged / enabledChanged
     * @param block 由 Widget new 出来的块，所有权仍在 Widget（本类只借用指针）
     * @note 若已有图，会 emit requestReprocess()
     */
    void addBlock(BaseBlock *block);

    /**
     * @brief 从链中移除块（不断开之外不做 delete）
     * @note 若已有图，会 emit requestReprocess()
     */
    void removeBlock(BaseBlock *block);

    /** @brief 当前处理链（只读），顺序即执行顺序 */
    const QList<BaseBlock*>& blocks() const { return m_blocks; }

    /**
     * @brief 设置本次 reprocess 使用的 ROI
     * @param roi shape==None 表示全图
     */
    void setRoi(const RoiInfo &roi);
    RoiInfo roi() const { return m_roi; }

    /**
     * @brief 核心入口：按链顺序执行所有启用块
     * @return 本次墙钟耗时（毫秒）；无图返回 0
     * @note 结束后 emit processingFinished
     */
    qint64 reprocess();

    /** @brief 上一次 reprocess 的耗时缓存 */
    qint64 lastElapsedMs() const { return m_lastElapsed; }

signals:
    /**
     * @brief 一次 reprocess 完成
     * @param elapsedMs 耗时毫秒，Widget 用来更新 label_3
     */
    void processingFinished(qint64 elapsedMs);

    /**
     * @brief 请求 Widget：请先同步最新 ROI，再调用 reprocess()
     *
     * 触发条件：加块、删块、某块参数变、某块开关变。
     * 不要在本类内部直接 reprocess，否则可能用到过期的 m_roi。
     */
    void requestReprocess();

private slots:
    /** 块 emit paramsChanged → 转成 requestReprocess */
    void onBlockParamsChanged();
    /** 块 emit enabledChanged → 转成 requestReprocess */
    void onBlockEnabledChanged(bool enabled);

private:
    QPixmap m_original;          ///< 原始图像（只读数据源）
    QPixmap m_result;            ///< 流水线输出
    QList<BaseBlock*> m_blocks;  ///< 处理链，顺序敏感
    RoiInfo m_roi;               ///< 当前 ROI；由 Widget 在重算前写入
    qint64 m_lastElapsed = 0;    ///< 最近一次耗时
};

#endif // IMAGEPROCESSOR_H
