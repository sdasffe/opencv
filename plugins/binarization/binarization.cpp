/**
 * @file binarization.cpp
 * @brief 范围二值化实现
 *
 * BinarizationBlock::process → applyRangeThreshold（经 RoiProcess）
 */

#include "binarization.h"

namespace BinarizationAlgorithm {

/** @brief 双阈值范围分割：像素 ∈ [lower, upper] → 255，否则 0 */
cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper)
{
    if (src.empty()) return cv::Mat();
    cv::Mat mask;
    cv::inRange(src, lower, upper, mask);                            // 输出 CV_8UC1
    return mask;
}

} // namespace BinarizationAlgorithm
