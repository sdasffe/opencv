#ifndef BINARIZATION_H
#define BINARIZATION_H

#include "opencv2/opencv.hpp"

/**
 * @file binarization.h
 * @brief 二值化算法集合（纯算法，无 UI 依赖）
 *
 * 【在整条链路中的位置】
 *   BinarizationBlock → applyRangeThreshold（经 RoiProcess::apply）
 *   Widget「Otsu」按钮 → OtsuAlgorithm 算阈值 → 写回块参数后重算
 *
 * 新流水线优先 RoiProcess + applyRangeThreshold；本头文件仍保留带 mask 的 ROI 专用接口。
 */
namespace BinarizationAlgorithm {

/** 与 OpenCV cv::threshold 对应的阈值模式 */
enum class ThresholdType {
    Binary,      // 正向二值化：灰度 > T 时置为 maxValue，否则为 0
    BinaryInv,   // 反向二值化：灰度 > T 时置为 0，否则为 maxValue
    Trunc,       // 截断：灰度 > T 时置为 T，否则保持原值
    ToZero,      // 阈值化到零：灰度 > T 保持原值，否则置 0
    ToZeroInv    // 反向阈值化到零：灰度 > T 置 0，否则保持原值
};

int toCvType(ThresholdType type);  // 将 ThresholdType 枚举映射为 OpenCV THRESH_* 常量

cv::Mat applyThreshold(const cv::Mat &src,           // 输入：单通道灰度或已转灰度图
                       double thresh,                  // 阈值 T（0~255）
                       double maxValue = 255.0,        // 超过阈值时写入的值，通常 255
                       ThresholdType type = ThresholdType::Binary);  // 二值化模式

cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper);
// 范围二值化：灰度 ∈ [lower, upper] → 255，否则 0；输出 CV_8UC1 蒙版式二值图

cv::Mat applyMaskedRangeThreshold(const cv::Mat &src,  // 输入：BGR 或灰度，ROI 外像素原样保留
                                  const cv::Mat &mask,  // 单通道 CV_8UC1，与 src 同尺寸，255=处理区域
                                  int lower,            // 灰度下限（含）
                                  int upper);           // 灰度上限（含）

cv::Mat applyRoiRangeThreshold(const cv::Mat &src,      // 输入：BGR 或灰度
                               const cv::Rect &roi,     // 轴对齐矩形 ROI（图像坐标，自动裁到边界内）
                               int lower,               // 灰度下限
                               int upper);              // 灰度上限

cv::Mat applyEllipseRoiRangeThreshold(const cv::Mat &src,           // 输入：BGR 或灰度
                                      const cv::Rect &boundingRect, // 椭圆外接矩形（与 Qt 椭圆图元 rect 对应）
                                      int lower,                    // 灰度下限
                                      int upper);                   // 灰度上限

cv::Mat applyRotatedRoiRangeThreshold(const cv::Mat &src,            // 输入：BGR 或灰度
                                      const cv::Point2f &center,     // 旋转矩形中心（图像坐标）
                                      const cv::Size2f &size,        // 宽、高（全尺寸，非半轴）
                                      float angleDeg,                // 旋转角（度，与 Qt 图元顺时针约定一致）
                                      int lower,                     // 灰度下限
                                      int upper);                    // 灰度上限

} // namespace BinarizationAlgorithm

#endif // BINARIZATION_H
