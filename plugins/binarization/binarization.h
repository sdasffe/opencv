#ifndef BINARIZATION_H
#define BINARIZATION_H

#include "opencv2/opencv.hpp"

/**
 * @file binarization.h
 * @brief 范围二值化（纯算法，无 UI）
 *
 * BinarizationBlock::process → RoiProcess::apply → applyRangeThreshold
 * ROI 由 RoiProcess 统一处理，本模块只做全图范围阈值。
 */
namespace BinarizationAlgorithm {

/** @brief 灰度 ∈ [lower, upper] → 255，否则 0；输出 CV_8UC1 */
cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper);

} // namespace BinarizationAlgorithm

#endif // BINARIZATION_H
