#include "pseudocolor.h"

namespace PseudoColorAlgorithm {

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
