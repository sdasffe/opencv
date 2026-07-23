#ifndef GLCM_H
#define GLCM_H

#include "opencv2/opencv.hpp"

/**
 * @file glcm.h
 * @brief GLCM 纹理特征：纯分析，不修改图像像素
 *
 * GlcmBlock::process → compute；对 0/45/90/135° 四方向平均。
 */
namespace GlcmAlgorithm {

struct Features {
    double contrast = 0;                                               // 对比度：灰度差加权
    double correlation = 0;                                            // 相关性
    double energy = 0;                                                 // 能量（ASM）
    double homogeneity = 0;                                            // 均匀性 / IDM
    double entropy = 0;                                                // 熵：无序程度
    double asmValue = 0;                                               // 角二阶矩（与 energy 同义）
    double dissimilarity = 0;                                          // 相异性
};

/**
 * @param srcBgr BGR 或灰度；@param levels 量化级 2~64；@param distance 像素对距离
 * @return 四方向平均 Features；空图全零
 */
Features compute(const cv::Mat &srcBgr, int levels = 32, int distance = 1);

} // namespace GlcmAlgorithm

#endif // GLCM_H
