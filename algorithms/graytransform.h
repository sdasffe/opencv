#ifndef GRAYTRANSFORM_H
#define GRAYTRANSFORM_H

#include "opencv2/opencv.hpp"

namespace GrayTransformAlgorithm {

enum class Type {
    ToGray,
    BrightContrast,
    Invert,
    Log,
    Gamma,
    Equalize,
    Normalize
};

/**
 * @param brightness -100~100
 * @param contrast   0~300（100=原样）
 * @param gamma      0.1~3.0 量级，传入百分数时由调用方换算
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type,
              int brightness, int contrast, double gamma);

} // namespace GrayTransformAlgorithm

#endif // GRAYTRANSFORM_H
