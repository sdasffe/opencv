/**
 * @file binarization.cpp
 * @brief 二值化算法实现 —— 阈值分割与 ROI 内范围阈值
 *
 * 【在整条链路中的位置】
 *   BinarizationBlock::process → 本命名空间函数
 *   Widget Otsu 按钮 → OtsuAlgorithm（见 otsu.cpp）算阈值 → 再调 applyRangeThreshold 等
 *
 * 【与 RoiProcess 的分工】
 *   新流水线优先用 RoiProcess::apply + applyRangeThreshold（整图 inRange）
 *   本文件仍保留 applyMaskedRangeThreshold / apply*Roi* 等「自带 mask」接口，
 *   供旧代码或需要精确控制 mask 的场景使用
 *
 * 【输入约定】
 *   多数函数接受 BGR 或灰度；范围阈值会先转灰度再 inRange
 */

#include "binarization.h"

#include <cmath>
#include <vector>
#include <algorithm>

namespace BinarizationAlgorithm {

/**
 * @brief 将本项目的 ThresholdType 映射为 OpenCV THRESH_* 标志
 *
 * 谁调用：applyThreshold
 * 为什么封装：UI/块层用枚举，算法层统一转 cv 常量，避免魔法数字散落
 */
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

/**
 * @brief 单阈值二值化（cv::threshold 封装）
 *
 * @param src      单通道灰度或已转灰度的图
 * @param thresh   阈值 T
 * @param maxValue 超过阈值时写入的值（通常 255）
 * @param type     二值化模式（正向/反向/截断等）
 * @return 与 src 同尺寸的二值/灰度结果
 */
cv::Mat applyThreshold(const cv::Mat &src, double thresh,
                       double maxValue, ThresholdType type)
{
    if (src.empty()) return cv::Mat();
    cv::Mat dst;
    cv::threshold(src, dst, thresh, maxValue, toCvType(type));
    return dst;
}

/**
 * @brief 双阈值范围分割（像素 ∈ [lower, upper] → 255，否则 0）
 *
 * 谁调用：BinarizationBlock 在 RoiProcess lambda 内
 * 输出：CV_8UC1 蒙版式二值图
 */
cv::Mat applyRangeThreshold(const cv::Mat &src, int lower, int upper)
{
    if (src.empty()) return cv::Mat();
    cv::Mat mask;
    cv::inRange(src, lower, upper, mask);
    return mask;
}

/**
 * @brief 仅在 mask==255 的像素上应用范围阈值，其余保持原图
 *
 * @param src   原图（BGR 或灰度）
 * @param mask  CV_8UC1，与 src 同尺寸
 * @param lower 灰度下限
 * @param upper 灰度上限
 *
 * 流程：
 *   1. result = src.clone()           // 默认全保留
 *   2. gray = 转灰度
 *   3. binary = inRange(gray, lower, upper)
 *   4. 逐像素：mask==0 跳过；mask!=0 处写黑/白
 *
 * 为什么手写循环而不是 bitwise：
 *   需要在 mask 区域内写「可视化」的 0/255（BGR 三通道同步），逻辑更直观
 */
cv::Mat applyMaskedRangeThreshold(const cv::Mat &src,
                                  const cv::Mat &mask,
                                  int lower,
                                  int upper)
{
    if (src.empty() || mask.empty()) return src.clone();
    // 尺寸/类型不对则安全退回原图，避免越界
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
                    continue; // ROI 外：保持 result 里原像素
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

/**
 * @brief 轴对齐矩形 ROI 内的范围阈值
 *
 * 先根据 roi 矩形生成 mask，再委托 applyMaskedRangeThreshold
 */
cv::Mat applyRoiRangeThreshold(const cv::Mat &src,
                               const cv::Rect &roi,
                               int lower,
                               int upper)
{
    if (src.empty()) return cv::Mat();

    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
    // & 运算：裁到图像边界内，防止 ROI 超出图像
    cv::Rect valid = roi & cv::Rect(0, 0, src.cols, src.rows);
    if (valid.empty())
        return src.clone();
    mask(valid).setTo(255);
    return applyMaskedRangeThreshold(src, mask, lower, upper);
}

/**
 * @brief 椭圆 ROI 内的范围阈值
 *
 * @param boundingRect 椭圆外接矩形（与 Qt 椭圆图元 rect 对应）
 */
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

/**
 * @brief 旋转矩形 ROI 内的范围阈值
 *
 * @param center   旋转矩形中心（图像坐标）
 * @param size     宽、高（全尺寸，非半轴）
 * @param angleDeg 旋转角（度），与 Qt 图元顺时针约定一致
 *
 * 为什么不用 cv::RotatedRect 直接画：
 *   OpenCV RotatedRect 角度约定与 Qt QGraphicsItem::rotation 可能不一致；
 *   手动算本地四角 → 旋转 → 平移，与 roiprocess.cpp / Widget 侧一致，避免错位。
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

    // OpenCV RotatedRect 角度：与 Qt 的 rotation（顺时针为正？）
    // Qt setRotation：顺时针为正；OpenCV RotatedRect：逆时针为正（旧约定）。
    // 实际用多边形顶点更稳：按 Qt 角度（顺时针）自己算四个角点。
    const float rad = angleDeg * static_cast<float>(CV_PI) / 180.f;
    const float c = std::cos(rad);
    const float s = std::sin(rad);
    const float hw = size.width / 2.f;
    const float hh = size.height / 2.f;

    // 以中心为原点的四个角（未旋转）
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
