#ifndef FILTER_H
#define FILTER_H

#include "opencv2/opencv.hpp"

/**
 * @file filter.h
 * @brief 空间滤波与边缘检测 —— 平滑与梯度算子
 *
 * FilterBlock::process → FilterAlgorithm::apply；经 RoiProcess 可局部滤波。
 * 平滑类支持 iterations；边缘类只跑 1 次，输出统一 BGR。
 */
namespace FilterAlgorithm {

enum class Type {
    Mean,                                                              // 均值（盒式）模糊
    Gaussian,                                                          // 高斯模糊
    Median,                                                            // 中值滤波，去椒盐
    Sobel,                                                             // Sobel 梯度幅值
    Laplacian,                                                         // Laplacian 二阶导
    Prewitt,                                                           // Prewitt 3×3 梯度
    Roberts                                                            // Roberts 2×2 交叉梯度
};

/**
 * @param srcBgr 输入 BGR；@param type 算法；@param kx,ky 核（中值仅用 kx）；@param iterations 次数
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type, int kx, int ky, int iterations); // Block 经 RoiProcess 调用

} // namespace FilterAlgorithm

#endif // FILTER_H
