#ifndef FILTER_H
#define FILTER_H

#include "opencv2/opencv.hpp"

namespace FilterAlgorithm {

enum class Type {
    Mean,
    Gaussian,
    Median,
    Sobel,
    Laplacian,
    Prewitt,
    Roberts
};

cv::Mat apply(const cv::Mat &srcBgr, Type type, int kx, int ky, int iterations);

} // namespace FilterAlgorithm

#endif // FILTER_H
