#ifndef GLCM_H
#define GLCM_H

#include "opencv2/opencv.hpp"

/**
 * @file glcm.h
 * @brief 灰度共生矩阵（GLCM）纹理特征 —— 纯分析，不修改图像
 *
 * 【在整条链路中的位置】
 *   GlcmBlock::process → GlcmAlgorithm::compute(当前图, levels, distance)
 *   → 返回 Haralick 统计量供块 UI 显示，流水线图像本身不变
 *
 * 默认：灰度量化 levels、距离 distance，对 0/45/90/135° 四个方向取平均。
 */
namespace GlcmAlgorithm {

struct Features {
    double contrast = 0;      ///< 对比度
    double correlation = 0;   ///< 相关性
    double energy = 0;        ///< 能量（ASM）
    double homogeneity = 0;   ///< 均匀性（IDM）
    double entropy = 0;       ///< 熵
    double asmValue = 0;      ///< 角二阶矩（与 energy 同义，分开显示便于教学）
    double dissimilarity = 0; ///< 相异性
};

/**
 * @param srcBgr   输入 BGR 或灰度（内部会转灰度并量化）
 * @param levels   灰度量化级数（2~64，越大越慢、纹理更细）
 * @param distance 像素对水平/垂直/对角偏移（通常 1）
 * @return         四方向平均后的 Features；空图返回全零
 */
Features compute(const cv::Mat &srcBgr, int levels = 32, int distance = 1);

} // namespace GlcmAlgorithm

#endif // GLCM_H
