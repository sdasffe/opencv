#include "binarization.h"

namespace BinarizationAlgorithm {

int toCvType(ThresholdType type)
{
    switch (type) {
    case ThresholdType::Binary:    return cv::THRESH_BINARY;
    case ThresholdType::BinaryInv: return cv::THRESH_BINARY_INV;
    case ThresholdType::Trunc:     return cv::THRESH_TRUNC;
    case ThresholdType::ToZero:    return cv::THRESH_TOZERO;
    case ThresholdType::ToZeroInv: return cv::THRESH_TOZERO_INV;
    default:                       return cv::THRESH_BINARY;
    }
}

cv::Mat applyThreshold(const cv::Mat &src, double thresh,
                       double maxValue, ThresholdType type)
{
    if (src.empty()) return cv::Mat();

    cv::Mat dst;
    cv::threshold(src, dst, thresh, maxValue, toCvType(type));
    return dst;
}

cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper)
{
    if (src.empty()) return cv::Mat();

    // 区间内 = 255（白），区间外 = 0（黑）
    cv::Mat mask;
    cv::inRange(src, lower, upper, mask);
    return mask;
}

cv::Mat applyRoiRangeThreshold(const cv::Mat &src,
                               const cv::Rect &roi,
                               int lower,
                               int upper)
{
    if (src.empty()) return cv::Mat();

    cv::Mat result = src.clone();

    // 裁剪 ROI 到图像范围内
    cv::Rect validRoi = roi & cv::Rect(0, 0, src.cols, src.rows);
    if (validRoi.empty()) return result;

    // 提取 ROI 区域的灰度图
    cv::Mat roiMat = src(validRoi);
    cv::Mat grayRoi;
    if (roiMat.channels() == 3) {
        cv::cvtColor(roiMat, grayRoi, cv::COLOR_BGR2GRAY);
    } else {
        grayRoi = roiMat;
    }

    // 生成二值化 mask
    cv::Mat mask;
    cv::inRange(grayRoi, lower, upper, mask);

    // 将 ROI 区域替换为黑白效果
    for (int y = 0; y < validRoi.height; y++) {
        const uchar *maskRow = mask.ptr<uchar>(y);
        cv::Vec3b *resultRow = result.ptr<cv::Vec3b>(validRoi.y + y);

        for (int x = 0; x < validRoi.width; x++) {
            if (maskRow[x] > 0) {
                resultRow[validRoi.x + x] = cv::Vec3b(255, 255, 255); // 白
            } else {
                resultRow[validRoi.x + x] = cv::Vec3b(0, 0, 0); // 黑
            }
        }
    }

    return result;
}

} // namespace BinarizationAlgorithm
