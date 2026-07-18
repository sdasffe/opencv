#include "imageprocessor.h"

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
{
}

void ImageProcessor::setOriginalImage(const QPixmap &pixmap)
{
    m_original = pixmap;
    m_result = pixmap;
}

void ImageProcessor::addBlock(BaseBlock *block)
{
    if (!block) return;

    m_blocks.append(block);
    connect(block, &BaseBlock::paramsChanged,
            this, &ImageProcessor::onBlockParamsChanged);
    connect(block, &BaseBlock::enabledChanged,
            this, &ImageProcessor::onBlockEnabledChanged);

    if (hasImage())
        emit requestReprocess();
}

void ImageProcessor::removeBlock(BaseBlock *block)
{
    if (!block) return;

    disconnect(block, nullptr, this, nullptr);
    m_blocks.removeOne(block);

    if (hasImage())
        emit requestReprocess();
}

void ImageProcessor::setRoi(const RoiInfo &roi)
{
    m_roi = roi;
}

qint64 ImageProcessor::reprocess()
{
    if (!hasImage()) return 0;

    TimeMeasurer tm("处理链", false);

    QPixmap current = m_original;
    for (BaseBlock *block : m_blocks) {
        if (!block->isEnabled()) continue;
        current = block->process(current, m_roi);
    }

    m_result = current;
    m_lastElapsed = tm.elapsedMs();
    emit processingFinished(m_lastElapsed);
    return m_lastElapsed;
}

void ImageProcessor::onBlockParamsChanged()
{
    if (hasImage())
        emit requestReprocess();
}

void ImageProcessor::onBlockEnabledChanged(bool /*enabled*/)
{
    if (hasImage())
        emit requestReprocess();
}
