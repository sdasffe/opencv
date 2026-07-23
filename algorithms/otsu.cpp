/**
 * @file otsu.cpp
 * @brief Otsu 自动阈值：只返回整数 T，不输出二值图
 */

#include "otsu.h"

namespace OtsuAlgorithm {

/**
 * @brief 对灰度图算 Otsu 阈值
 * @return 0~255；空图返回 128
 */
int calculateThreshold(const cv::Mat &grayMat)
{
    if (grayMat.empty()) return 128;                                 // 无数据给默认中值

    cv::Mat binary;                                                  // THRESH_OTSU 仍会写输出图，可丢弃
    double otsuThresh = cv::threshold(grayMat, binary, 0, 255,
                                       cv::THRESH_BINARY | cv::THRESH_OTSU); // 返回值即最优 T
    return static_cast<int>(otsuThresh);
}

/**
 * @brief BGR 先转灰度再 Otsu；供 Widget::wireBinarizationOtsu 使用
 */
int calculateThresholdFromBGR(const cv::Mat &bgrMat)
{
    if (bgrMat.empty()) return 128;

    cv::Mat gray;
    cv::cvtColor(bgrMat, gray, cv::COLOR_BGR2GRAY);                  // 大津法在灰度直方图上做
    return calculateThreshold(gray);
}

} // namespace OtsuAlgorithm
