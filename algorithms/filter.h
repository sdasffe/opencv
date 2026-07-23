#ifndef FILTER_H
#define FILTER_H

#include "opencv2/opencv.hpp"

/**
 * @file filter.h
 * @brief 空间滤波与边缘检测 —— 平滑与梯度算子
 *
 * 【在整条链路中的位置】
 *   FilterBlock::process → FilterAlgorithm::apply
 *   经 RoiProcess::apply 后可在 ROI 内局部滤波
 *
 * 平滑类（Mean/Gaussian/Median）支持 iterations 叠加；
 * 边缘类（Sobel 等）内部只执行一遍，输出统一为 BGR 便于显示。
 */
namespace FilterAlgorithm {

/** 滤波/边缘算法类型 */
enum class Type {
    Mean,       ///< 均值（盒式）模糊
    Gaussian,   ///< 高斯模糊
    Median,     ///< 中值滤波，去椒盐噪点
    Sobel,      ///< Sobel 梯度幅值
    Laplacian,  ///< Laplacian 二阶导
    Prewitt,    ///< Prewitt 3×3 梯度
    Roberts     ///< Roberts 2×2 交叉梯度
};

/**
 * @param srcBgr     输入 BGR
 * @param type       算法类型
 * @param kx, ky     核尺寸（中值滤波仅用 kx）
 * @param iterations 迭代次数（边缘类实际只跑 1 次）
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type, int kx, int ky, int iterations);

} // namespace FilterAlgorithm

#endif // FILTER_H
