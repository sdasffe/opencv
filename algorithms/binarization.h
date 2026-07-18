#ifndef BINARIZATION_H
#define BINARIZATION_H

#include "opencv2/opencv.hpp"

/**
 * @brief 二值化算法集合（纯算法，无 UI 依赖）
 */
namespace BinarizationAlgorithm {

enum class ThresholdType {
    Binary,
    BinaryInv,
    Trunc,
    ToZero,
    ToZeroInv
};

int toCvType(ThresholdType type);

cv::Mat applyThreshold(const cv::Mat &src,
                       double thresh,
                       double maxValue = 255.0,
                       ThresholdType type = ThresholdType::Binary);

/** 范围二值化：区间内白，区间外黑 */
cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper);

/**
 * @brief 仅对 mask 为非 0 的像素做范围二值化，其余保持原图
 * @param src BGR 图
 * @param mask 单通道，与 src 同尺寸，255=处理区域
 */
cv::Mat applyMaskedRangeThreshold(const cv::Mat &src,
                                  const cv::Mat &mask,
                                  int lower,
                                  int upper);

/** 轴对齐矩形 ROI */
cv::Mat applyRoiRangeThreshold(const cv::Mat &src,
                               const cv::Rect &roi,
                               int lower,
                               int upper);

/** 椭圆 ROI（外接矩形） */
cv::Mat applyEllipseRoiRangeThreshold(const cv::Mat &src,
                                      const cv::Rect &boundingRect,
                                      int lower,
                                      int upper);

/** 旋转矩形 ROI（角度单位：度，与 OpenCV RotatedRect 一致） */
cv::Mat applyRotatedRoiRangeThreshold(const cv::Mat &src,
                                      const cv::Point2f &center,
                                      const cv::Size2f &size,
                                      float angleDeg,
                                      int lower,
                                      int upper);

} // namespace BinarizationAlgorithm

#endif // BINARIZATION_H
