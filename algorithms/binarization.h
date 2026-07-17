#ifndef BINARIZATION_H
#define BINARIZATION_H

#include "opencv2/opencv.hpp"

/**
 * @brief 二值化算法集合
 *
 * 纯算法封装，无 UI 依赖。
 * 支持多种二值化模式，所有函数输入输出均为 cv::Mat。
 */
namespace BinarizationAlgorithm {

// 二值化模式枚举
enum class ThresholdType {
    Binary,       // THRESH_BINARY：大于阈值为最大值，否则为0
    BinaryInv,    // THRESH_BINARY_INV：大于阈值为0，否则为最大值
    Trunc,        // THRESH_TRUNC：大于阈值截断为阈值，否则不变
    ToZero,       // THRESH_TOZERO：大于阈值不变，否则为0
    ToZeroInv     // THRESH_TOZERO_INV：大于阈值为0，否则不变
};

/**
 * @brief 全局二值化
 * @param src 输入图像（建议灰度图）
 * @param thresh 阈值
 * @param maxValue 最大值（通常 255）
 * @param type 二值化类型
 * @return 二值化后的图像
 */
cv::Mat applyThreshold(const cv::Mat &src,
                       double thresh,
                       double maxValue = 255.0,
                       ThresholdType type = ThresholdType::Binary);

/**
 * @brief 范围二值化（区间内为白，区间外为黑）
 * @param src 输入灰度图
 * @param lower 下限阈值
 * @param upper 上限阈值
 * @return 二值化图像（CV_8UC1，区间内=255，区间外=0）
 */
cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper);

/**
 * @brief 对图像指定矩形区域做二值化，其余区域保持原样
 * @param src 输入彩色图（BGR）
 * @param roi 感兴趣区域
 * @param lower 下限阈值
 * @param upper 上限阈值
 * @return 处理后的彩色图（ROI内二值化黑白，ROI外保持彩色）
 */
cv::Mat applyRoiRangeThreshold(const cv::Mat &src,
                               const cv::Rect &roi,
                               int lower,
                               int upper);

/**
 * @brief 将 OpenCV 原生阈值类型转换为 int
 */
int toCvType(ThresholdType type);

} // namespace BinarizationAlgorithm

#endif // BINARIZATION_H
