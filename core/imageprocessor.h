#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QPixmap>
#include <QList>
#include "../blocks/baseblock.h"
#include "../roi/roiinfo.h"
#include "../utils/timemeasurer.h"

/**
 * @brief 图像处理调度引擎
 *
 * 不主动在 ROI 拖动时重算；由 Widget 在「应用处理」或参数变化时
 * 先 setRoi() 再 reprocess()。
 */
class ImageProcessor : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessor(QObject *parent = nullptr);

    void setOriginalImage(const QPixmap &pixmap);
    QPixmap originalImage() const { return m_original; }
    QPixmap resultImage() const { return m_result; }
    bool hasImage() const { return !m_original.isNull(); }

    void addBlock(BaseBlock *block);
    void removeBlock(BaseBlock *block);
    const QList<BaseBlock*>& blocks() const { return m_blocks; }

    void setRoi(const RoiInfo &roi);
    RoiInfo roi() const { return m_roi; }

    qint64 reprocess();
    qint64 lastElapsedMs() const { return m_lastElapsed; }

signals:
    void processingFinished(qint64 elapsedMs);
    /** 参数/块变化时请求外部同步 ROI 后再处理 */
    void requestReprocess();

private slots:
    void onBlockParamsChanged();
    void onBlockEnabledChanged(bool enabled);

private:
    QPixmap m_original;
    QPixmap m_result;
    QList<BaseBlock*> m_blocks;
    RoiInfo m_roi;
    qint64 m_lastElapsed = 0;
};

#endif // IMAGEPROCESSOR_H
