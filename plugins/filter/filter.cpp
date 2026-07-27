/**
 * @file filter.cpp
 * @brief 空间滤波与边缘检测 —— 平滑与梯度算子
 *
 * FilterBlock::process → FilterAlgorithm::apply
 * 平滑类可 iterations 叠加；边缘类只执行一遍后 break。
 * 边缘结果转 8bit BGR，便于后续块与 QPixmap 统一。
 */

#include "filter.h"
#include <algorithm>

namespace FilterAlgorithm {

/** @brief 核尺寸：奇数且上限 31（与 UI SpinBox 一致） */
static int oddSize(int v)
{
    v = std::max(1, v);
    if (v % 2 == 0) ++v;
    return std::min(v, 31);                                          // 过大核会卡顿
}

/** @brief 边缘幅值（浮点/16S）→ 可显示的 8bit BGR */
static cv::Mat edgeToBgr(const cv::Mat &edge)
{
    cv::Mat absEdge, u8, bgr;
    cv::convertScaleAbs(edge, absEdge);                              // 取绝对值并缩放到 8U 量级
    if (absEdge.type() != CV_8U)
        absEdge.convertTo(u8, CV_8U);                                // 兜底转 8U
    else
        u8 = absEdge;
    cv::cvtColor(u8, bgr, cv::COLOR_GRAY2BGR);                       // 流水线约定三通道
    return bgr;
}

/**
 * @brief 应用滤波或边缘检测
 * @param srcBgr 输入 BGR；@param type 算法；@param kx,ky 核；@param iterations 次数
 */
cv::Mat apply(const cv::Mat &srcBgr, Type type, int kx, int ky, int iterations)
{
    if (srcBgr.empty()) return srcBgr.clone();
    iterations = std::max(1, iterations);
    const int ox = oddSize(kx);                                      // 合法奇数核宽
    const int oy = oddSize(ky);                                      // 合法奇数核高

    cv::Mat current = srcBgr.clone();
    for (int i = 0; i < iterations; ++i) {
        cv::Mat dst;
        switch (type) {
        case Type::Mean:
            cv::blur(current, dst, cv::Size(ox, oy));                // 盒式均值模糊
            current = dst;
            break;
        case Type::Gaussian:
            cv::GaussianBlur(current, dst, cv::Size(ox, oy), 0);     // sigma=0 由核尺寸推导
            current = dst;
            break;
        case Type::Median:
            cv::medianBlur(current, dst, ox);                        // 中值只支持方核，用 ox
            current = dst;
            break;
        case Type::Sobel: {
            cv::Mat gray, gx, gy, gxF, gyF, mag;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            cv::Sobel(gray, gx, CV_16S, 1, 0, ox >= 3 ? 3 : 1);      // ∂x
            cv::Sobel(gray, gy, CV_16S, 0, 1, ox >= 3 ? 3 : 1);      // ∂y
            gx.convertTo(gxF, CV_32F);
            gy.convertTo(gyF, CV_32F);
            cv::magnitude(gxF, gyF, mag);                            // √(Gx²+Gy²)
            current = edgeToBgr(mag);
            break;
        }
        case Type::Laplacian: {
            cv::Mat gray, lap;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            cv::Laplacian(gray, lap, CV_16S, ox >= 3 ? 3 : 1);       // 二阶导数
            current = edgeToBgr(lap);
            break;
        }
        case Type::Prewitt: {
            cv::Mat gray, gx, gy, mag;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            cv::Mat kx = (cv::Mat_<float>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1);
            cv::Mat ky = (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1);
            cv::filter2D(gray, gx, CV_32F, kx);                      // 固定 3×3 Prewitt
            cv::filter2D(gray, gy, CV_32F, ky);
            cv::magnitude(gx, gy, mag);
            current = edgeToBgr(mag);
            break;
        }
        case Type::Roberts: {
            cv::Mat gray, gx, gy, mag;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            cv::Mat kx = (cv::Mat_<float>(2, 2) << 1, 0, 0, -1);
            cv::Mat ky = (cv::Mat_<float>(2, 2) << 0, 1, -1, 0);
            cv::filter2D(gray, gx, CV_32F, kx);                      // 2×2 Roberts 交叉
            cv::filter2D(gray, gy, CV_32F, ky);
            cv::magnitude(gx, gy, mag);
            current = edgeToBgr(mag);
            break;
        }
        }
        if (type == Type::Sobel || type == Type::Laplacian
            || type == Type::Prewitt || type == Type::Roberts)
            break;                                                   // 边缘类只跑一遍
    }
    return current;
}

} // namespace FilterAlgorithm
