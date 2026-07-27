#ifndef OTSU_H
#define OTSU_H

#include "opencv2/opencv.hpp"

/**
 * @file otsu.h
 * @brief Otsu 大津法 —— 为二值化块提供自动阈值（只返回 T，不输出二值图）
 *
 * BinarizationBlock「Otsu」→ calculateThresholdFromBGR → 写回 SpinBox → 重算。
 */
namespace OtsuAlgorithm {

/** @param grayMat CV_8UC1；@return 最优阈值 0~255，空图返回 128 */
int calculateThreshold(const cv::Mat &grayMat);                        // 灰度图直接算 T

/** @param bgrMat CV_8UC3；内部转灰度再算；供 Widget::wireBinarizationOtsu */
int calculateThresholdFromBGR(const cv::Mat &bgrMat);                  // 彩色入口

} // namespace OtsuAlgorithm

#endif // OTSU_H
