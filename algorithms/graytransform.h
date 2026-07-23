#ifndef GRAYTRANSFORM_H
#define GRAYTRANSFORM_H

#include "opencv2/opencv.hpp"

/**
 * @file graytransform.h
 * @brief 灰度与点运算 —— 像素级变换（不使用空间邻域）
 *
 * 【在整条链路中的位置】
 *   GrayTransformBlock::process → GrayTransformAlgorithm::apply
 *
 * 与 filter.h 的区别：每像素只依赖自身（或全局直方图），非卷积/形态学。
 */
namespace GrayTransformAlgorithm {

/** 点运算类型 */
enum class Type {
    ToGray,          ///< BGR → 灰度再扩成三通道显示
    BrightContrast,  ///< 线性亮度/对比度
    Invert,          ///< 反色
    Log,             ///< 对数变换 + 拉伸
    Gamma,           ///< 伽马校正
    Equalize,        ///< 全局直方图均衡
    Normalize        ///< MINMAX 归一化到 0~255
};

/**
 * @param srcBgr     输入 BGR
 * @param type       变换类型
 * @param brightness 亮度偏移 −100~100（BrightContrast）
 * @param contrast   对比度 0~300，100=原样（BrightContrast）
 * @param gamma      伽马指数 0.1~3.0 量级（Gamma）
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type,
              int brightness, int contrast, double gamma);

} // namespace GrayTransformAlgorithm

#endif // GRAYTRANSFORM_H
