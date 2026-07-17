#include "imageconverter.h"

cv::Mat ImageConverter::pixmapToMatRGB(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(image.height(), image.width(), CV_8UC3,
                const_cast<uchar*>(image.constBits()),
                image.bytesPerLine());
    return mat.clone(); // 深拷贝，脱离 QImage 生命周期
}

cv::Mat ImageConverter::pixmapToMatGray(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);
    cv::Mat mat(image.height(), image.width(), CV_8UC1,
                const_cast<uchar*>(image.constBits()),
                image.bytesPerLine());
    return mat.clone();
}

QPixmap ImageConverter::matToPixmap(const cv::Mat &mat)
{
    if (mat.empty()) return QPixmap();

    QImage::Format format;
    cv::Mat temp;

    switch (mat.type()) {
    case CV_8UC1:
        format = QImage::Format_Grayscale8;
        temp = mat;
        break;
    case CV_8UC3:
        // OpenCV 默认 BGR，转 RGB
        cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
        format = QImage::Format_RGB888;
        break;
    case CV_8UC4:
        cv::cvtColor(mat, temp, cv::COLOR_BGRA2RGBA);
        format = QImage::Format_RGBA8888;
        break;
    default:
        // 其他格式统一转 RGB
        if (mat.channels() == 1) {
            temp = mat;
            format = QImage::Format_Grayscale8;
        } else {
            cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        }
        break;
    }

    QImage image(temp.data, temp.cols, temp.rows,
                 static_cast<int>(temp.step), format);
    return QPixmap::fromImage(image.copy()); // copy 做深拷贝
}

cv::Mat ImageConverter::imageToMat(const QImage &image, bool rgb)
{
    QImage::Format fmt = rgb ? QImage::Format_RGB888 : QImage::Format_Grayscale8;
    QImage converted = image.convertToFormat(fmt);

    int type = rgb ? CV_8UC3 : CV_8UC1;
    cv::Mat mat(converted.height(), converted.width(), type,
                const_cast<uchar*>(converted.constBits()),
                converted.bytesPerLine());
    return mat.clone();
}
