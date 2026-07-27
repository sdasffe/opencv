#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include "../blocksdk/blocksdk_global.h"
#include <QPixmap>
#include <QImage>
#include "opencv2/opencv.hpp"

/**
 * @file imageconverter.h
 * @brief QPixmap / QImage 与 cv::Mat 互转工具
 */
class BLOCKSDK_EXPORT ImageConverter
{
public:
    static cv::Mat pixmapToMatRGB(const QPixmap &pixmap);
    static cv::Mat pixmapToMatGray(const QPixmap &pixmap);
    static QPixmap matToPixmap(const cv::Mat &mat);
    static cv::Mat imageToMat(const QImage &image, bool rgb = true);

private:
    ImageConverter() = delete;
};

#endif // IMAGECONVERTER_H
