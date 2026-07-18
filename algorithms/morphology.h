#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "opencv2/opencv.hpp"

namespace MorphologyAlgorithm {

enum class Op {
    Dilate,
    Erode,
    Open,
    Close
};

/** 形态学处理，kx/ky 为核尺寸（自动修正为奇数），iterations 为次数 */
cv::Mat apply(const cv::Mat &srcBgr, Op op, int kx, int ky, int iterations);

} // namespace MorphologyAlgorithm

#endif // MORPHOLOGY_H
