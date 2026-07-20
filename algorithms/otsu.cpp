/**
 * @file otsu.cpp
 * @brief Otsu 自动阈值 —— 为二值化块提供「一键算阈值」
 *
 * 【在整条链路中的位置】
 *   BinarizationBlock 上「Otsu」按钮 → Widget::wireBinarizationOtsu
 *   → OtsuAlgorithm::calculateThresholdFromBGR(当前图)
 *   → 把算出的阈值写回 SpinBox，触发 paramsChanged 重算
 *
 * 【算法原理简述】
 *   Otsu 假设灰度直方图双峰，遍历所有候选阈值 T，
 *   使类间方差最大（或类内方差最小）的 T 即为最优分割点。
 *   本实现直接调用 cv::threshold(..., THRESH_OTSU)，由 OpenCV 完成计算。
 *
 * 【不负责】
 *   不执行二值化输出图，只返回整数阈值供 UI 使用
 */

#include "otsu.h"

namespace OtsuAlgorithm {

/**
 * @brief 对灰度图计算 Otsu 阈值
 *
 * @param grayMat 单通道 CV_8U
 * @return 阈值 0~255；空图返回默认 128
 *
 * cv::threshold 在带 THRESH_OTSU 时：
 *   - 仍会生成 binary 输出（此处可丢弃）
 *   - 返回值即为 Otsu 阈值
 */
int calculateThreshold(const cv::Mat &grayMat)
{
    if (grayMat.empty()) return 128;

    cv::Mat binary;
    double otsuThresh = cv::threshold(grayMat, binary, 0, 255,
                                       cv::THRESH_BINARY | cv::THRESH_OTSU);
    return static_cast<int>(otsuThresh);
}

/**
 * @brief 对 BGR 彩色图先转灰度再 Otsu
 *
 * 谁调用：Widget 在用户点击 Otsu 且当前预览图为 BGR 时
 * 为什么分两层：块内 Mat 已是 BGR，UI 层不必重复 cvtColor
 */
int calculateThresholdFromBGR(const cv::Mat &bgrMat)
{
    if (bgrMat.empty()) return 128;

    cv::Mat gray;
    cv::cvtColor(bgrMat, gray, cv::COLOR_BGR2GRAY);
    return calculateThreshold(gray);
}

} // namespace OtsuAlgorithm
