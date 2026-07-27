#ifndef GRAYTRANSFORM_H
#define GRAYTRANSFORM_H

#include "opencv2/opencv.hpp"

/**
 * @file graytransform.h
 * @brief 灰度与点运算 —— 像素级变换（不使用空间邻域）
 *
 * GrayTransformBlock::process → GrayTransformAlgorithm::apply
 * 与 filter 区别：每像素只依赖自身或全局直方图。
 */
namespace GrayTransformAlgorithm {

enum class Type {
    ToGray,                                                            // BGR→灰度再扩成三通道显示
    BrightContrast,                                                    // 线性亮度/对比度
    Invert,                                                            // 反色
    Log,                                                               // 对数变换 + 拉伸
    Gamma,                                                             // 伽马校正
    Equalize,                                                          // 全局直方图均衡
    Normalize                                                          // MINMAX 归一化到 0~255
};

/**
 * @param brightness 亮度偏移；@param contrast 对比度%（100=原样）；@param gamma 伽马指数
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type,
              int brightness, int contrast, double gamma);             // Block 经 RoiProcess 调用

} // namespace GrayTransformAlgorithm

#endif // GRAYTRANSFORM_H
