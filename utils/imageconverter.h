#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QPixmap>
#include <QImage>
#include "opencv2/opencv.hpp"

/**
 * @brief QPixmap / QImage 与 cv::Mat 互转工具
 *
 * 纯静态工具类，统一处理图像格式转换，避免各模块重复写转换代码。
 * 所有转换都做深拷贝，返回独立内存，不依赖原对象生命周期。
 */
class ImageConverter
{
public:
    // ========== QPixmap → cv::Mat ==========

    /**
     * @brief QPixmap 转 RGB 格式 cv::Mat (CV_8UC3)
     * @param pixmap 输入 Qt 像素图
     * @return OpenCV Mat，RGB 三通道
     */
    static cv::Mat pixmapToMatRGB(const QPixmap &pixmap);

    /**
     * @brief QPixmap 转灰度格式 cv::Mat (CV_8UC1)
     * @param pixmap 输入 Qt 像素图
     * @return OpenCV Mat，单通道灰度
     */
    static cv::Mat pixmapToMatGray(const QPixmap &pixmap);

    // ========== cv::Mat → QPixmap ==========

    /**
     * @brief cv::Mat 转 QPixmap
     * @param mat 输入 OpenCV Mat（支持灰度、RGB、RGBA）
     * @return Qt 像素图
     */
    static QPixmap matToPixmap(const cv::Mat &mat);

    // ========== QImage → cv::Mat 辅助 ==========

    /**
     * @brief QImage 转 cv::Mat（深拷贝）
     * @param image 输入 Qt 图像
     * @param format 目标格式（默认 RGB）
     * @return OpenCV Mat
     */
    static cv::Mat imageToMat(const QImage &image, bool rgb = true);

private:
    ImageConverter() = delete; // 纯静态类，禁止实例化
};

#endif // IMAGECONVERTER_H
