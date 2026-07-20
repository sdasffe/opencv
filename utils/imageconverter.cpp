/**
 * @file imageconverter.cpp
 * @brief Qt 图像与 OpenCV Mat 互转 —— 整条流水线的「格式桥梁」
 *
 * 【在整条链路中的位置】
 *   每个 BaseBlock::process() 入口：QPixmap → Mat
 *   每个 BaseBlock::process() 出口：Mat → QPixmap
 *   Widget 加载/保存图片时也会用到
 *
 * 【为什么单独抽一层】
 *   1. QImage 与 cv::Mat 内存布局、通道顺序不同（Qt 常用 RGB，OpenCV 常用 BGR）
 *   2. 避免各算法块重复写 convertToFormat / clone 逻辑
 *   3. 统一深拷贝策略，防止 Mat 指向已销毁的 QImage 缓冲区
 *
 * 【通道约定】
 *   pixmapToMatRGB → RGB 三通道（块内通常再 cvtColor 到 BGR 给算法）
 *   matToPixmap    → 输入假定为 OpenCV BGR/BGRA/灰度，输出 Qt 可显示的 QPixmap
 */

#include "imageconverter.h"

/**
 * @brief QPixmap → cv::Mat(RGB, CV_8UC3)
 *
 * 谁调用：各 Block::process、部分 Widget 逻辑
 * 输入：Qt 像素图（任意内部格式）
 * 输出：独立内存的 RGB Mat
 *
 * 步骤：
 *   1. toImage() 得到 QImage
 *   2. convertToFormat(RGB888) 统一为 24bit RGB
 *   3. 用 QImage 缓冲区构造 Mat 视图（零拷贝）
 *   4. clone() 深拷贝 —— 必须！否则 QImage 析构后 Mat 悬空
 */
cv::Mat ImageConverter::pixmapToMatRGB(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    // Mat 构造函数：高度、宽度、类型、数据指针、每行字节数（可能有对齐 padding）
    cv::Mat mat(image.height(), image.width(), CV_8UC3,
                const_cast<uchar*>(image.constBits()),
                image.bytesPerLine());
    return mat.clone(); // 深拷贝，脱离 QImage 生命周期
}

/**
 * @brief QPixmap → cv::Mat 灰度 (CV_8UC1)
 *
 * 适用：只需单通道的算法或统计（如 Otsu 预览）
 * 同样必须 clone() 保证 Mat 生命周期独立
 */
cv::Mat ImageConverter::pixmapToMatGray(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);
    cv::Mat mat(image.height(), image.width(), CV_8UC1,
                const_cast<uchar*>(image.constBits()),
                image.bytesPerLine());
    return mat.clone();
}

/**
 * @brief cv::Mat → QPixmap
 *
 * 谁调用：各 Block::process 返回前、Widget 刷新预览
 * 输入：OpenCV Mat（常见 CV_8UC1 / CV_8UC3 / CV_8UC4）
 * 输出：Qt 可绘制的 QPixmap
 *
 * 为什么 BGR 要转 RGB：
 *   OpenCV 读图/算子默认 BGR，Qt QImage::Format_RGB888 期望 R-G-B 顺序，
 *   不转换则红蓝通道对调。
 */
QPixmap ImageConverter::matToPixmap(const cv::Mat &mat)
{
    if (mat.empty()) return QPixmap();

    QImage::Format format;
    cv::Mat temp; // 可能经过 cvtColor，需要临时 Mat

    switch (mat.type()) {
    case CV_8UC1:
        // 灰度：直接包装，无需通道交换
        format = QImage::Format_Grayscale8;
        temp = mat;
        break;
    case CV_8UC3:
        // OpenCV 默认 BGR，转 RGB 给 Qt
        cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
        format = QImage::Format_RGB888;
        break;
    case CV_8UC4:
        cv::cvtColor(mat, temp, cv::COLOR_BGRA2RGBA);
        format = QImage::Format_RGBA8888;
        break;
    default:
        // 非常见类型：尽量转成可显示格式
        if (mat.channels() == 1) {
            temp = mat;
            format = QImage::Format_Grayscale8;
        } else {
            cv::cvtColor(mat, temp, cv::COLOR_BGR2RGB);
            format = QImage::Format_RGB888;
        }
        break;
    }

    // 用 temp 的数据构造 QImage（浅拷贝），再 copy() 深拷贝给 QPixmap
    QImage image(temp.data, temp.cols, temp.rows,
                 static_cast<int>(temp.step), format);
    return QPixmap::fromImage(image.copy()); // copy 做深拷贝
}

/**
 * @brief QImage → cv::Mat（通用入口）
 *
 * @param rgb true → RGB888 / CV_8UC3；false → 灰度 / CV_8UC1
 *
 * 与 pixmapToMat* 类似，但调用方已有 QImage 时使用（如从文件解码后）
 */
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
