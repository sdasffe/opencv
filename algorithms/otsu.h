#ifndef OTSU_H
#define OTSU_H

#include "opencv2/opencv.hpp"

/**
 * @brief Otsu 大津法阈值计算
 *
 * 纯算法封装，无 UI 依赖，输入灰度图自动计算最佳二值化阈值。
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
