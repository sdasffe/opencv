/**
 * @file graytransform.cpp
 * @brief 灰度与点运算 —— 像素级变换（非邻域滤波）
 *
 * GrayTransformBlock::process → GrayTransformAlgorithm::apply
 * 多数模式仍输出 BGR 三通道，方便后续块统一处理。
 */

#include "graytransform.h"

#include <cmath>

namespace GrayTransformAlgorithm {

/** @brief 单通道则转 BGR，已是 BGR 则原样返回 */
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
 * @param brightness 亮度偏移 beta；@param contrast 对比度%（100=原图）；@param gamma 伽马指数
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type,
              int brightness, int contrast, double gamma)
{
    if (srcBgr.empty()) return srcBgr.clone();

    switch (type) {
    case Type::ToGray: {
        cv::Mat gray, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);              // 压成灰度
        cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);                 // 仍输出三通道「灰 RGB」
        return bgr;
    }
    case Type::BrightContrast: {
        const double alpha = std::max(0.0, contrast / 100.0);        // alpha = contrast/100
        cv::Mat dst;
        srcBgr.convertTo(dst, -1, alpha, brightness);                // dst = src*alpha + beta
        return dst;
    }
    case Type::Invert: {
        cv::Mat dst;
        cv::bitwise_not(srcBgr, dst);                                // 255−x
        return dst;
    }
    case Type::Log: {
        cv::Mat gray, f, out, u8, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        gray.convertTo(f, CV_32F, 1.0 / 255.0);                      // 归一化到 [0,1]
        cv::log(f + 1.0f, out);                                      // log(0) 无定义，故 +1
        double maxv;
        cv::minMaxLoc(out, nullptr, &maxv);
        if (maxv > 1e-6)
            out = out * (255.0 / maxv);                              // 拉伸到满量程
        out.convertTo(u8, CV_8U);
        cv::cvtColor(u8, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    case Type::Gamma: {
        gamma = std::max(0.05, gamma);                               // 防止 gamma→0 数值问题
        cv::Mat f, out;
        srcBgr.convertTo(f, CV_32F, 1.0 / 255.0);
        cv::pow(f, gamma, out);                                      // (I/255)^gamma
        out.convertTo(out, CV_8U, 255.0);                            // 拉回 0~255
        return ensureBgr(out);
    }
    case Type::Equalize: {
        cv::Mat gray, eq, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, eq);                                  // 全局直方图均衡
        cv::cvtColor(eq, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    case Type::Normalize: {
        cv::Mat gray, normed, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        cv::normalize(gray, normed, 0, 255, cv::NORM_MINMAX);        // 线性拉伸到 0~255
        normed.convertTo(normed, CV_8U);
        cv::cvtColor(normed, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    }
    return srcBgr.clone();                                           // 未知类型：原样返回
}

} // namespace GrayTransformAlgorithm
