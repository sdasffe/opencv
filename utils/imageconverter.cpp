/**
 * @file imageconverter.cpp
 * @brief Qt 图像与 OpenCV Mat 互转 —— 整条流水线的「格式桥梁」
 *
 * 通道约定：pixmapToMatRGB 输出 RGB；matToPixmap 输入假定为 BGR/BGRA/灰度。
 */

#include "imageconverter.h"

/** @brief QPixmap 转 RGB 三通道 cv::Mat (CV_8UC3) */
cv::Mat ImageConverter::pixmapToMatRGB(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888); // 统一为 24bit RGB
    cv::Mat mat(image.height(), image.width(), CV_8UC3,
                const_cast<uchar*>(image.constBits()),
                image.bytesPerLine()); // 用 QImage 缓冲区构造 Mat 视图（零拷贝）
    return mat.clone(); // 深拷贝，脱离 QImage 生命周期
}

/** @brief QPixmap 转单通道灰度 cv::Mat (CV_8UC1) */
cv::Mat ImageConverter::pixmapToMatGray(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8); // 统一为 8bit 灰度
    cv::Mat mat(image.height(), image.width(), CV_8UC1,
                const_cast<uchar*>(image.constBits()),
                image.bytesPerLine()); // 用 QImage 缓冲区构造 Mat 视图
    return mat.clone(); // 深拷贝，保证 Mat 独立存活
}

/** @brief cv::Mat 转 QPixmap，自动处理 BGR→RGB 通道交换 */
QPixmap ImageConverter::matToPixmap(const cv::Mat &mat)
{
    if (mat.empty()) return QPixmap(); // 空 Mat 返回空 QPixmap

    QImage::Format format; // 目标 QImage 像素格式
    cv::Mat temp;          // 可能经过 cvtColor 的临时 Mat

    switch (mat.type()) {
    case CV_8UC1:
        format = QImage::Format_Grayscale8; // 灰度无需通道交换
        temp = mat;
        break;
    case CV_8UC3:
        cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB); // OpenCV BGR → Qt RGB
        format = QImage::Format_RGB888;
        break;
    case CV_8UC4:
        cv::cvtColor(mat, temp, cv::COLOR_BGRA2RGBA); // 带 Alpha 通道
        format = QImage::Format_RGBA8888;
        break;
    default:
        if (mat.channels() == 1) { // 非常见类型：单通道按灰度处理
            temp = mat;
            format = QImage::Format_Grayscale8;
        } else { // 多通道默认转 RGB
            cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        }
        break;
    }

    QImage image(temp.data, temp.cols, temp.rows,
                 static_cast<int>(temp.step), format); // 浅拷贝包装 temp 数据
    return QPixmap::fromImage(image.copy()); // copy 深拷贝后生成 QPixmap
}

/** @brief QImage 转 cv::Mat（通用入口，调用方已有 QImage 时使用） */
cv::Mat ImageConverter::imageToMat(const QImage &image, bool rgb)
{
    QImage::Format fmt = rgb ? QImage::Format_RGB888 : QImage::Format_Grayscale8; // 按参数选格式
    QImage converted = image.convertToFormat(fmt); // 统一像素格式

    int type = rgb ? CV_8UC3 : CV_8UC1; // 对应 OpenCV 类型
    cv::Mat mat(converted.height(), converted.width(), type,
                const_cast<uchar*>(converted.constBits()),
                converted.bytesPerLine()); // 构造 Mat 视图
    return mat.clone(); // 深拷贝
}
