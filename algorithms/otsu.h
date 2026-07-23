#ifndef OTSU_H
#define OTSU_H

#include "opencv2/opencv.hpp"

/**
 * @file otsu.h
 * @brief Otsu 大津法阈值计算 —— 为二值化块提供自动阈值
 *
 * 【在整条链路中的位置】
 *   BinarizationBlock「Otsu」→ calculateThresholdFromBGR → 写回 SpinBox → 重算二值化
 *   只返回整数阈值，不直接输出二值图（二值化由 BinarizationAlgorithm 完成）
 */
namespace OtsuAlgorithm {

/**
 * @brief 计算 Otsu 最优阈值
 * @param grayMat 输入灰度图（CV_8UC1）
 * @return 最佳阈值（0-255），若输入为空返回 128
 */
int calculateThreshold(const cv::Mat &grayMat);

/**
 * @brief 从彩色 BGR 图计算 Otsu 阈值（内部自动转灰度）
 * @param bgrMat 输入彩色图（CV_8UC3）
 * @return 最佳阈值（0-255）
 */
int calculateThresholdFromBGR(const cv::Mat &bgrMat);

} // namespace OtsuAlgorithm

#endif // OTSU_H
