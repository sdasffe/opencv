/**
 * @file binarization.cpp
 * @brief 二值化算法实现 —— 阈值分割与 ROI 内范围阈值
 *
 * BinarizationBlock::process → applyRangeThreshold（经 RoiProcess）
 * 本文件仍保留带 mask 的 ROI 专用接口，供精确控制 mask 的场景。
 */

#include "binarization.h"

#include <cmath>
#include <vector>
#include <algorithm>

namespace BinarizationAlgorithm {

/** @brief 本项目 ThresholdType → OpenCV THRESH_* */
int toCvType(ThresholdType type)
{
    switch (type) {
    case ThresholdType::Binary:    return cv::THRESH_BINARY;
    case ThresholdType::BinaryInv: return cv::THRESH_BINARY_INV;
    case ThresholdType::Trunc:     return cv::THRESH_TRUNC;
    case ThresholdType::ToZero:    return cv::THRESH_TOZERO;
    case ThresholdType::ToZeroInv: return cv::THRESH_TOZERO_INV;
    default:                       return cv::THRESH_BINARY;         // 未知回退正向二值
    }
}

/** @brief 单阈值二值化（cv::threshold 封装） */
cv::Mat applyThreshold(const cv::Mat &src, double thresh,
                       double maxValue, ThresholdType type)
{
    if (src.empty()) return cv::Mat();                               // 空图返回空
    cv::Mat dst;
    cv::threshold(src, dst, thresh, maxValue, toCvType(type));
    return dst;
}

/** @brief 双阈值范围分割：像素 ∈ [lower, upper] → 255，否则 0 */
cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper)
{
    if (src.empty()) return cv::Mat();
    cv::Mat mask;
    cv::inRange(src, lower, upper, mask);                            // 输出 CV_8UC1
    return mask;
}

/**
 * @brief 仅在 mask==255 像素上应用范围阈值，其余保持原图
 * 手写循环以便在 mask 内写可视化 0/255（BGR 三通道同步）
 */
cv::Mat applyMaskedRangeThreshold(const cv::Mat &src,
                                  const cv::Mat &mask,
                                  int lower,
                                  int upper)
{
    if (src.empty() || mask.empty()) return src.clone();
    if (mask.size() != src.size() || mask.type() != CV_8UC1)
        return src.clone();                                          // 尺寸/类型不对则安全退回

    cv::Mat result = src.clone();                                    // 默认全保留

    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src;

    cv::Mat binary;
    cv::inRange(gray, lower, upper, binary);                         // 全图范围二值

    for (int y = 0; y < src.rows; ++y) {
        const uchar *maskRow = mask.ptr<uchar>(y);
        const uchar *binRow = binary.ptr<uchar>(y);
        if (result.channels() == 3) {
            cv::Vec3b *outRow = result.ptr<cv::Vec3b>(y);
            for (int x = 0; x < src.cols; ++x) {
                if (maskRow[x] == 0)
                    continue;                                        // ROI 外：保持原像素
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

/** @brief 轴对齐矩形 ROI 内范围阈值：先造 mask 再委托 applyMasked* */
cv::Mat applyRoiRangeThreshold(const cv::Mat &src,
                               const cv::Rect &roi,
                               int lower,
                               int upper)
{
    if (src.empty()) return cv::Mat();

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::Rect valid = roi & cv::Rect(0, 0, src.cols, src.rows);       // 裁到图像边界
    if (valid.empty())
        return src.clone();
    mask(valid).setTo(255);
    return applyMaskedRangeThreshold(src, mask, lower, upper);
}

/** @brief 椭圆 ROI 内范围阈值；boundingRect 与 Qt 椭圆图元 rect 对应 */
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
    cv::ellipse(mask, center, axes, 0.0, 0.0, 360.0, cv::Scalar(255), cv::FILLED); // 填充椭圆
    return applyMaskedRangeThreshold(src, mask, lower, upper);
}

/**
 * @brief 旋转矩形 ROI 内范围阈值
 * 手动算四角再 fillConvexPoly，与 roiprocess / Widget 角度约定一致
 */
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

    const float rad = angleDeg * static_cast<float>(CV_PI) / 180.f;
    const float c = std::cos(rad);
    const float s = std::sin(rad);
    const float hw = size.width / 2.f;
    const float hh = size.height / 2.f;

    cv::Point2f local[4] = {
        {-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}                   // 未旋转的四角（中心原点）
    };
    std::vector<cv::Point> poly;
    poly.reserve(4);
    for (const auto &p : local) {
        const float x = p.x * c - p.y * s;                           // 顺时针角旋转
        const float y = p.x * s + p.y * c;
        poly.emplace_back(cv::Point(cvRound(center.x + x), cvRound(center.y + y)));
    }

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
    cv::fillConvexPoly(mask, poly, cv::Scalar(255));                 // 填充旋转矩形
    return applyMaskedRangeThreshold(src, mask, lower, upper);
}

} // namespace BinarizationAlgorithm
