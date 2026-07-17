#include "otsu.h"

namespace OtsuAlgorithm {

int calculateThreshold(const cv::Mat &grayMat)
{
    if (grayMat.empty()) return 128;

    cv::Mat binary;
    double otsuThresh = cv::threshold(grayMat, binary, 0, 255,
                                       cv::THRESH_BINARY | cv::THRESH_OTSU);
    return static_cast<int>(otsuThresh);
}

int calculateThresholdFromBGR(const cv::Mat &bgrMat)
{
    if (bgrMat.empty()) return 128;

    cv::Mat gray;
    cv::cvtColor(bgrMat, gray, cv::COLOR_BGR2GRAY);
    return calculateThreshold(gray);
}

} // namespace OtsuAlgorithm
