#include "morphology.h"
#include <algorithm>

namespace MorphologyAlgorithm {

static int oddSize(int v)
{
    v = std::max(1, v);
    if (v % 2 == 0) ++v;
    return v;
}

cv::Mat apply(const cv::Mat &srcBgr, Op op, int kx, int ky, int iterations)
{
    if (srcBgr.empty()) return srcBgr.clone();
    iterations = std::max(1, iterations);
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT, cv::Size(oddSize(kx), oddSize(ky)));

    cv::Mat dst;
    switch (op) {
    case Op::Dilate:
        cv::dilate(srcBgr, dst, kernel, cv::Point(-1, -1), iterations);
        break;
    case Op::Erode:
        cv::erode(srcBgr, dst, kernel, cv::Point(-1, -1), iterations);
        break;
    case Op::Open:
        cv::morphologyEx(srcBgr, dst, cv::MORPH_OPEN, kernel,
                         cv::Point(-1, -1), iterations);
        break;
    case Op::Close:
        cv::morphologyEx(srcBgr, dst, cv::MORPH_CLOSE, kernel,
                         cv::Point(-1, -1), iterations);
        break;
    }
    return dst;
}

} // namespace MorphologyAlgorithm
