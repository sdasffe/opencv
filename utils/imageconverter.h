#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QPixmap>
#include <QImage>
#include "opencv2/opencv.hpp"

/**
 * @file imageconverter.h
 * @brief QPixmap / QImage 与 cv::Mat 互转工具
 *
 * 纯静态工具类，统一处理图像格式转换；所有转换均深拷贝，返回独立内存。
 */
class ImageConverter
{
public:
    static cv::Mat pixmapToMatRGB(const QPixmap &pixmap);  // QPixmap → RGB 格式 cv::Mat (CV_8UC3)
    static cv::Mat pixmapToMatGray(const QPixmap &pixmap); // QPixmap → 灰度格式 cv::Mat (CV_8UC1)
    static QPixmap matToPixmap(const cv::Mat &mat);        // cv::Mat → QPixmap（支持灰度/RGB/RGBA）
    static cv::Mat imageToMat(const QImage &image, bool rgb = true); // QImage → cv::Mat 深拷贝；rgb=true 为 RGB888，false 为灰度

private:
    ImageConverter() = delete; // 纯静态类，禁止实例化
};

#endif // IMAGECONVERTER_H
