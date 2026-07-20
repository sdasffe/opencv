/**
 * @file filter.cpp
 * @brief 空间滤波与边缘检测 —— 平滑与梯度算子
 *
 * 【在整条链路中的位置】
 *   FilterBlock::process → FilterAlgorithm::apply
 *
 * 【两类算法】
 *   1. 平滑：Mean / Gaussian / Median —— 可 iterations 次叠加
 *   2. 边缘：Sobel / Laplacian / Prewitt / Roberts —— 只执行一遍（循环内 break）
 *
 * 【输出约定】
 *   平滑类保持 BGR；边缘类转灰度梯度幅值后再 GRAY2BGR，便于流水线统一显示
 */

#include "filter.h"
#include <algorithm>

namespace FilterAlgorithm {

/**
 * @brief 核尺寸：奇数且上限 31（与 UI SpinBox 一致，防止过大核卡顿）
 */
static int oddSize(int v)
{
    v = std::max(1, v);
    if (v % 2 == 0) ++v;
    return std::min(v, 31);
}

/**
 * @brief 将边缘检测结果（浮点/16S 幅值）转为可显示的 BGR 图
 *
 * 步骤：convertScaleAbs → 必要时转 CV_8U → cvtColor GRAY2BGR
 * 为什么：后续 Block 与 QPixmap 转换假定 8bit BGR
 */
static cv::Mat edgeToBgr(const cv::Mat &edge)
{
    cv::Mat absEdge, u8, bgr;
    cv::convertScaleAbs(edge, absEdge);
    if (absEdge.type() != CV_8U)
        absEdge.convertTo(u8, CV_8U);
    else
        u8 = absEdge;
    cv::cvtColor(u8, bgr, cv::COLOR_GRAY2BGR);
    return bgr;
}

/**
 * @brief 应用滤波或边缘检测
 *
 * @param srcBgr     输入 BGR
 * @param type       算法类型
 * @param kx, ky     核尺寸（中值滤波仅用 ox）
 * @param iterations 迭代次数（边缘类实际只跑 1 次）
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type, int kx, int ky, int iterations)
{
    if (srcBgr.empty()) return srcBgr.clone();
    iterations = std::max(1, iterations);
    const int ox = oddSize(kx);
    const int oy = oddSize(ky);

    cv::Mat current = srcBgr.clone();
    for (int i = 0; i < iterations; ++i) {
        cv::Mat dst;
        switch (type) {
        case Type::Mean:
            // 盒式模糊，各通道独立
            cv::blur(current, dst, cv::Size(ox, oy));
            current = dst;
            break;
        case Type::Gaussian:
            // sigma=0 时由核尺寸自动推导
            cv::GaussianBlur(current, dst, cv::Size(ox, oy), 0);
            current = dst;
            break;
        case Type::Median:
            cv::medianBlur(current, dst, ox); // 中值只支持方核，用 ox
            current = dst;
            break;
        case Type::Sobel: {
            cv::Mat gray, gx, gy, gxF, gyF, mag;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            // ksize 3 或 1：小核更灵敏，大核更平滑
            cv::Sobel(gray, gx, CV_16S, 1, 0, ox >= 3 ? 3 : 1);
            cv::Sobel(gray, gy, CV_16S, 0, 1, ox >= 3 ? 3 : 1);
            gx.convertTo(gxF, CV_32F);
            gy.convertTo(gyF, CV_32F);
            cv::magnitude(gxF, gyF, mag); // |Gx| 与 |Gy| 的合成梯度
            current = edgeToBgr(mag);
            break;
        }
        case Type::Laplacian: {
            cv::Mat gray, lap;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            cv::Laplacian(gray, lap, CV_16S, ox >= 3 ? 3 : 1);
            current = edgeToBgr(lap);
            break;
        }
        case Type::Prewitt: {
            cv::Mat gray, gx, gy, mag;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            // 固定 3×3 Prewitt 核，用 filter2D 卷积
            cv::Mat kx = (cv::Mat_<float>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
            cv::Mat ky = (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);
            cv::filter2D(gray, gx, CV_32F, kx);
            cv::filter2D(gray, gy, CV_32F, ky);
            cv::magnitude(gx, gy, mag);
            current = edgeToBgr(mag);
            break;
        }
        case Type::Roberts: {
            cv::Mat gray, gx, gy, mag;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            // 2×2 Roberts 交叉梯度
            cv::Mat kx = (cv::Mat_<float>(2, 2) << 1, 0, 0, -1);
            cv::Mat ky = (cv::Mat_<float>(2, 2) << 0, 1, -1, 0);
            cv::filter2D(gray, gx, CV_32F, kx);
            cv::filter2D(gray, gy, CV_32F, ky);
            cv::magnitude(gx, gy, mag);
            current = edgeToBgr(mag);
            break;
        }
        }
        // 边缘类一般只做一遍（多次迭代无意义且会过度模糊）
        if (type == Type::Sobel || type == Type::Laplacian
            || type == Type::Prewitt || type == Type::Roberts)
            break;
    }
    return current;
}

} // namespace FilterAlgorithm
