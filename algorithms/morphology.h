#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "opencv2/opencv.hpp"

/**
 * @file morphology.h
 * @brief 形态学运算 —— 膨胀、腐蚀、开闭运算等
 *
 * 【在整条链路中的位置】
 *   MorphologyBlock::process → MorphologyAlgorithm::apply
 *   经 RoiProcess::apply 后可只作用于 ROI 区域
 *
 * 输入/输出均为 BGR，结构元素为轴对齐矩形核（MORPH_RECT）。
 */
namespace MorphologyAlgorithm {

/** 形态学算子类型（与 OpenCV morphologyEx 一一对应） */
enum class Op {
    Dilate,                  ///< 膨胀：加粗前景、填小缝
    Erode,                   ///< 腐蚀：去小噪点、细化
    Open,                    ///< 开运算：先腐蚀后膨胀，去小亮斑
    Close,                   ///< 闭运算：先膨胀后腐蚀，填小暗孔
    TopHat,                  ///< 顶帽：原图 − 开运算，提取比背景亮的小结构
    ButtonHat,               ///< 底帽：闭运算 − 原图，提取比背景暗的小结构
    MorphologicalGradient    ///< 形态学梯度：膨胀 − 腐蚀，突出边缘
};

/**
 * @param srcBgr     输入 BGR 图
 * @param op         运算类型
 * @param kx, ky     矩形结构元素宽、高（内部会修正为奇数）
 * @param iterations 重复次数
 */
cv::Mat apply(const cv::Mat &srcBgr, Op op, int kx, int ky, int iterations);

} // namespace MorphologyAlgorithm

#endif // MORPHOLOGY_H
