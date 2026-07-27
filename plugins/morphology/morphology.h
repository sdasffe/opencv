#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "opencv2/opencv.hpp"

/**
 * @file morphology.h
 * @brief 形态学运算 —— 膨胀、腐蚀、开闭、顶帽/底帽、梯度
 *
 * MorphologyBlock::process → MorphologyAlgorithm::apply
 * 输入/输出 BGR；结构元为轴对齐矩形核（MORPH_RECT）。
 */
namespace MorphologyAlgorithm {

enum class Op {
    Dilate,                                                            // 膨胀：加粗前景、填小缝
    Erode,                                                             // 腐蚀：去小噪点、细化
    Open,                                                              // 开：先腐蚀后膨胀，去小亮斑
    Close,                                                             // 闭：先膨胀后腐蚀，填小暗孔
    TopHat,                                                            // 顶帽：原图 − 开，提亮小结构
    ButtonHat,                                                         // 底帽：闭 − 原图，提暗小结构
    MorphologicalGradient                                              // 梯度：膨胀 − 腐蚀，突出边缘
};

/**
 * @param srcBgr 输入 BGR；@param op 算子；@param kx,ky 核宽高（内部改奇数）；@param iterations 次数
 */
cv::Mat apply(const cv::Mat &srcBgr, Op op, int kx, int ky, int iterations); // Block 经 RoiProcess 调用

} // namespace MorphologyAlgorithm

#endif // MORPHOLOGY_H
