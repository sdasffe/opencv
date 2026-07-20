/**
 * @file graytransform.cpp
 * @brief 灰度与点运算 —— 像素级变换（非邻域滤波）
 *
 * 【在整条链路中的位置】
 *   GrayTransformBlock::process → GrayTransformAlgorithm::apply
 *
 * 【与 filter.cpp 的区别】
 *   本文件每个像素只依赖自身（或全局直方图），不使用空间邻域
 *
 * 【输出】
 *   多数模式返回 BGR 三通道（即使中间是灰度），方便后续块统一处理
 */

#include "graytransform.h"

#include <cmath>

namespace GrayTransformAlgorithm {

/**
 * @brief 若输入为单通道则转为 BGR，已是 BGR 则原样返回
 *
 * 伽马等运算可能得到单通道 Mat，统一成 BGR 避免 matToPixmap 分支混乱
 */
static cv::Mat ensureBgr(const cv::Mat &src)
{
    if (src.channels() == 1) {
        cv::Mat bgr;
        cv::cvtColor(src, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    return src;
}

/**
 * @brief 执行指定的灰度/点变换
 *
 * @param srcBgr     输入 BGR（部分模式会先转灰度）
 * @param type       变换类型
 * @param brightness 亮度偏移 beta（BrightContrast）
 * @param contrast   对比度百分比，100=原图（BrightContrast）
 * @param gamma      伽马指数（Gamma）
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type,
              int brightness, int contrast, double gamma)
{
    if (srcBgr.empty()) return srcBgr.clone();

    switch (type) {
    case Type::ToGray: {
        cv::Mat gray, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR); // 仍输出三通道「灰 RGB」
        return bgr;
    }
    case Type::BrightContrast: {
        // OpenCV: dst = src * alpha + beta
        // alpha = contrast/100, beta = brightness
        const double alpha = std::max(0.0, contrast / 100.0);
        cv::Mat dst;
        srcBgr.convertTo(dst, -1, alpha, brightness);
        return dst;
    }
    case Type::Invert: {
        cv::Mat dst;
        cv::bitwise_not(srcBgr, dst); // 255-x，逐位取反
        return dst;
    }
    case Type::Log: {
        // s = c * log(1 + I)，再归一化到 0~255
        cv::Mat gray, f, out, u8, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        gray.convertTo(f, CV_32F, 1.0 / 255.0); // 归一化到 [0,1]
        cv::log(f + 1.0f, out);                 // log(0) 无定义，故 +1
        double maxv;
        cv::minMaxLoc(out, nullptr, &maxv);
        if (maxv > 1e-6)
            out = out * (255.0 / maxv);         // 拉伸到满量程
        out.convertTo(u8, CV_8U);
        cv::cvtColor(u8, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    case Type::Gamma: {
        // O = (I/255)^gamma * 255
        gamma = std::max(0.05, gamma); // 防止 gamma→0 数值问题
        cv::Mat f, out;
        srcBgr.convertTo(f, CV_32F, 1.0 / 255.0);
        cv::pow(f, gamma, out);
        out.convertTo(out, CV_8U, 255.0);
        return ensureBgr(out);
    }
    case Type::Equalize: {
        // 全局直方图均衡，仅对灰度有意义
        cv::Mat gray, eq, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, eq);
        cv::cvtColor(eq, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    case Type::Normalize: {
        // 线性拉伸灰度到 0~255（MINMAX）
        cv::Mat gray, normed, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        cv::normalize(gray, normed, 0, 255, cv::NORM_MINMAX);
        normed.convertTo(normed, CV_8U);
        cv::cvtColor(normed, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    }
    return srcBgr.clone();
}

} // namespace GrayTransformAlgorithm
