#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QList>
#include <QRectF>
#include "../blocks/baseblock.h"
#include "../utils/timemeasurer.h"

/**
 * @brief 图像处理调度引擎
 *
 * 职责：
 * 1. 管理原始图像与处理结果
 * 2. 维护处理块列表，按顺序执行处理链
 * 3. 统一处理 ROI 坐标映射
 * 4. 统计处理耗时
 *
 * 与 UI 解耦：Widget 只负责把处理块加进来，具体调度由本类完成。
 */
class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessor(QObject *parent = nullptr);

    // ========== 图像管理 ==========

    /** 设置原始图像（处理链的输入） */
    void setOriginalImage(const QPixmap &pixmap);
    QPixmap originalImage() const { return m_original; }

    /** 获取当前处理后的结果图像 */
    QPixmap resultImage() const { return m_result; }

    /** 是否有有效图像 */
    bool hasImage() const { return !m_original.isNull(); }

    // ========== 处理块管理 ==========

    /** 添加处理块到链尾 */
    void addBlock(BaseBlock *block);

    /** 移除处理块 */
    void removeBlock(BaseBlock *block);

    /** 获取所有处理块 */
    const QList<BaseBlock*>& blocks() const { return m_blocks; }

    // ========== ROI 设置 ==========

    /** 设置当前 ROI（图像像素坐标系），空矩形表示全图处理 */
    void setRoiRect(const QRectF &roi);
    QRectF roiRect() const { return m_roi; }

    // ========== 执行处理 ==========

    /**
     * @brief 重新执行整个处理链
     * @return 处理耗时（毫秒）
     */
    qint64 reprocess();

    /** 获取上次处理耗时 */
    qint64 lastElapsedMs() const { return m_lastElapsed; }

signals:
    /** 处理完成，结果已更新 */
    void processingFinished(qint64 elapsedMs);

private slots:
    void onBlockParamsChanged();
    void onBlockEnabledChanged(bool enabled);

private:
    QPixmap m_original;
    QPixmap m_result;
    QList<BaseBlock*> m_blocks;
    QRectF m_roi;
    qint64 m_lastElapsed = 0;
};

#endif // IMAGEPROCESSOR_H
