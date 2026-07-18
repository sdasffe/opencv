#ifndef PSEUDOCOLOR_H
#define PSEUDOCOLOR_H

#include "opencv2/opencv.hpp"

namespace PseudoColorAlgorithm {

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

cv::Mat apply(const cv::Mat &srcBgr, Map map);

} // namespace PseudoColorAlgorithm

#endif // PSEUDOCOLOR_H
