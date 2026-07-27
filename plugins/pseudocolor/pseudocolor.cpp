/**
 * @file pseudocolor.cpp
 * @brief 伪彩色映射 —— 灰度强度 → 彩色显示
 *
 * PseudoColorBlock::process → PseudoColorAlgorithm::apply
 * 先转灰度，再 cv::applyColorMap 按查找表映射为 BGR。
 */

#include "pseudocolor.h"

namespace PseudoColorAlgorithm {

/** @brief 本项目 Map 枚举 → OpenCV COLORMAP_* 常量 */
static int toCvColormap(Map map)
{
    switch (map) {
    case Map::Jet:     return cv::COLORMAP_JET;
    case Map::Hot:     return cv::COLORMAP_HOT;
    case Map::Cool:    return cv::COLORMAP_COOL;
    case Map::Rainbow: return cv::COLORMAP_RAINBOW;
    case Map::Ocean:   return cv::COLORMAP_OCEAN;
    case Map::Summer:  return cv::COLORMAP_SUMMER;
    case Map::Winter:  return cv::COLORMAP_WINTER;
    case Map::Autumn:  return cv::COLORMAP_AUTUMN;
    case Map::Bone:    return cv::COLORMAP_BONE;
    case Map::Pink:    return cv::COLORMAP_PINK;
    }
    return cv::COLORMAP_JET;                                         // 未知枚举回退 Jet
}

/**
 * @brief 应用伪彩色映射
 * @param srcBgr 输入（BGR 或灰度）；@param map 色表类型
 * @return BGR 伪彩色图
 */
cv::Mat apply(const cv::Mat &srcBgr, Map map)
{
    if (srcBgr.empty()) return srcBgr.clone();                       // 空图直通

    cv::Mat gray, color;
    if (srcBgr.channels() == 3)
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);              // 彩色先压成单通道强度
    else
        gray = srcBgr;                                               // 已是灰度则直接用

    cv::applyColorMap(gray, color, toCvColormap(map));               // LUT 映射 → BGR 伪彩
    return color;
}

} // namespace PseudoColorAlgorithm
