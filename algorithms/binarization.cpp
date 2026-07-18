#include "binarization.h"

#include <cmath>
#include <vector>
#include <algorithm>

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
    cv::Mat mask;
    cv::inRange(src, lower, upper, mask);
    return mask;
}

cv::Mat applyMaskedRangeThreshold(const cv::Mat &src,
                                  const cv::Mat &mask,
                                  int lower,
                                  int upper)
{
    if (src.empty() || mask.empty()) return src.clone();
    if (mask.size() != src.size() || mask.type() != CV_8UC1)
        return src.clone();

    cv::Mat result = src.clone();

    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src;

    cv::Mat binary;
    cv::inRange(gray, lower, upper, binary);

    for (int y = 0; y < src.rows; ++y) {
        const uchar *maskRow = mask.ptr<uchar>(y);
        const uchar *binRow = binary.ptr<uchar>(y);
        if (result.channels() == 3) {
            cv::Vec3b *outRow = result.ptr<cv::Vec3b>(y);
            for (int x = 0; x < src.cols; ++x) {
                if (maskRow[x] == 0)
                    continue;
                outRow[x] = binRow[x] ? cv::Vec3b(255, 255, 255) : cv::Vec3b(0, 0, 0);
            }
        } else {
            uchar *outRow = result.ptr<uchar>(y);
            for (int x = 0; x < src.cols; ++x) {
                if (maskRow[x] == 0)
                    continue;
                outRow[x] = binRow[x] ? 255 : 0;
            }
        }
    }
    return result;
}

cv::Mat applyRoiRangeThreshold(const cv::Mat &src,
                               const cv::Rect &roi,
                               int lower,
                               int upper)
{
    if (src.empty()) return cv::Mat();

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::Rect valid = roi & cv::Rect(0, 0, src.cols, src.rows);
    if (valid.empty())
        return src.clone();
    mask(valid).setTo(255);
    return applyMaskedRangeThreshold(src, mask, lower, upper);
}

cv::Mat applyEllipseRoiRangeThreshold(const cv::Mat &src,
                                      const cv::Rect &boundingRect,
                                      int lower,
                                      int upper)
{
    if (src.empty()) return cv::Mat();

    cv::Rect valid = boundingRect & cv::Rect(0, 0, src.cols, src.rows);
    if (valid.empty())
        return src.clone();

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::Point center(valid.x + valid.width / 2, valid.y + valid.height / 2);
    cv::Size axes(std::max(1, valid.width / 2), std::max(1, valid.height / 2));
    cv::ellipse(mask, center, axes, 0.0, 0.0, 360.0, cv::Scalar(255), cv::FILLED);
    return applyMaskedRangeThreshold(src, mask, lower, upper);
}

cv::Mat applyRotatedRoiRangeThreshold(const cv::Mat &src,
                                      const cv::Point2f &center,
                                      const cv::Size2f &size,
                                      float angleDeg,
                                      int lower,
                                      int upper)
{
    if (src.empty()) return cv::Mat();
    if (size.width < 1.f || size.height < 1.f)
        return src.clone();

    // OpenCV RotatedRect 角度：与 Qt 的 rotation（顺时针为正？）
    // Qt setRotation：顺时针为正；OpenCV RotatedRect：逆时针为正（旧约定）。
    // 实际用多边形顶点更稳：按 Qt 角度（顺时针）自己算四个角点。
    const float rad = angleDeg * static_cast<float>(CV_PI) / 180.f;
    const float c = std::cos(rad);
    const float s = std::sin(rad);
    const float hw = size.width / 2.f;
    const float hh = size.height / 2.f;

    cv::Point2f local[4] = {
        {-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}
    };
    std::vector<cv::Point> poly;
    poly.reserve(4);
    for (const auto &p : local) {
        // Qt：y 向下，rotation 顺时针；二维旋转矩阵 [c -s; s c] 对顺时针角
        const float x = p.x * c - p.y * s;
        const float y = p.x * s + p.y * c;
        poly.emplace_back(cv::Point(cvRound(center.x + x), cvRound(center.y + y)));
    }

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::fillConvexPoly(mask, poly, cv::Scalar(255));
    return applyMaskedRangeThreshold(src, mask, lower, upper);
}

} // namespace BinarizationAlgorithm
