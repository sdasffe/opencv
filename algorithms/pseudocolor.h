#ifndef PSEUDOCOLOR_H
#define PSEUDOCOLOR_H

#include "opencv2/opencv.hpp"

/**
 * @file pseudocolor.h
 * @brief 伪彩色映射 —— 灰度强度映射为彩色显示
 *
 * 【在整条链路中的位置】
 *   PseudoColorBlock::process → PseudoColorAlgorithm::apply
 *
 * 典型用于热力图、深度图、强度图的可视化；内部先转灰度再 applyColorMap。
 */
namespace PseudoColorAlgorithm {

/** 内置色表（对应 OpenCV COLORMAP_*） */
enum class Map {
    Jet,
    Hot,
    Cool,
    Rainbow,
    Ocean,
    Summer,
    Winter,
    Autumn,
    Bone,
    Pink
};

/**
 * @param srcBgr 输入 BGR 或单通道灰度
 * @param map    色表类型
 * @return       BGR 伪彩色图
 */
cv::Mat apply(const cv::Mat &srcBgr, Map map);

} // namespace PseudoColorAlgorithm

#endif // PSEUDOCOLOR_H
