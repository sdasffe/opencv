/**
 * @file pseudocolor.cpp
 * @brief 伪彩色映射 —— 灰度强度 → 彩色显示
 *
 * 【在整条链路中的位置】
 *   PseudoColorBlock::process → PseudoColorAlgorithm::apply
 *
 * 【原理】
 *   先将输入转为单通道灰度（彩色输入则先 BGR2GRAY），
 *   再 cv::applyColorMap 按查找表映射为 BGR 伪彩色
 *
 * 【典型场景】
 *   热力图、深度图、医学/工业强度图的可视化
 */

#include "pseudocolor.h"

namespace PseudoColorAlgorithm {

/**
 * @brief 本项目 Map 枚举 → OpenCV COLORMAP_* 常量
 *
 * 封装原因：UI 与块层不直接依赖 cv::COLORMAP_JET 等魔法数
 */
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
    return cv::COLORMAP_JET;
}

/**
 * @brief 应用伪彩色映射
 *
 * @param srcBgr 输入（BGR 或已是灰度）
 * @param map    色表类型
 * @return BGR 伪彩色图（OpenCV applyColorMap 输出为 BGR）
 */
cv::Mat apply(const cv::Mat &srcBgr, Map map)
{
    if (srcBgr.empty()) return srcBgr.clone();

    cv::Mat gray, color;
    if (srcBgr.channels() == 3)
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
    else
        gray = srcBgr;

    cv::applyColorMap(gray, color, toCvColormap(map));
    return color; // BGR
}

} // namespace PseudoColorAlgorithm
