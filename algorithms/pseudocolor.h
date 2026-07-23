#ifndef PSEUDOCOLOR_H
#define PSEUDOCOLOR_H

#include "opencv2/opencv.hpp"

/**
 * @file pseudocolor.h
 * @brief 伪彩色：灰度强度 → OpenCV COLORMAP_* 彩色 BGR
 *
 * PseudoColorBlock::process → apply；内部先转灰度再上色。
 */
namespace PseudoColorAlgorithm {

/** 内置色表（对应 OpenCV COLORMAP_*） */
enum class Map {
    Jet,                                                               // 喷射：蓝→青→黄→红
    Hot,                                                               // 热力：黑→红→黄→白
    Cool,                                                              // 冷色青紫系
    Rainbow,                                                           // 彩虹
    Ocean,                                                             // 海洋蓝绿
    Summer,                                                            // 夏日绿黄
    Winter,                                                            // 冬日蓝青
    Autumn,                                                            // 秋日红黄
    Bone,                                                              // 骨质灰粉
    Pink                                                               // 粉红系
};

/**
 * @param srcBgr 输入 BGR 或单通道；@param map 色表
 * @return BGR 伪彩色图；空输入返回空 Mat
 */
cv::Mat apply(const cv::Mat &srcBgr, Map map);                         // Block 经 RoiProcess 调用

} // namespace PseudoColorAlgorithm

#endif // PSEUDOCOLOR_H
