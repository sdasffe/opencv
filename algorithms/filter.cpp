#include "filter.h"
#include <algorithm>

namespace FilterAlgorithm {

static int oddSize(int v)
{
    v = std::max(1, v);
    if (v % 2 == 0) ++v;
    return std::min(v, 31);
}

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
            cv::blur(current, dst, cv::Size(ox, oy));
            current = dst;
            break;
        case Type::Gaussian:
            cv::GaussianBlur(current, dst, cv::Size(ox, oy), 0);
            current = dst;
            break;
        case Type::Median:
            cv::medianBlur(current, dst, ox); // 中值只支持方核
            current = dst;
            break;
        case Type::Sobel: {
            cv::Mat gray, gx, gy, gxF, gyF, mag;
            cv::cvtColor(current, gray, cv::COLOR_BGR2GRAY);
            cv::Sobel(gray, gx, CV_16S, 1, 0, ox >= 3 ? 3 : 1);
            cv::Sobel(gray, gy, CV_16S, 0, 1, ox >= 3 ? 3 : 1);
            gx.convertTo(gxF, CV_32F);
            gy.convertTo(gyF, CV_32F);
            cv::magnitude(gxF, gyF, mag);
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
            cv::Mat kx = (cv::Mat_<float>(2, 2) << 1, 0, 0, -1);
            cv::Mat ky = (cv::Mat_<float>(2, 2) << 0, 1, -1, 0);
            cv::filter2D(gray, gx, CV_32F, kx);
            cv::filter2D(gray, gy, CV_32F, ky);
            cv::magnitude(gx, gy, mag);
            current = edgeToBgr(mag);
            break;
        }
        }
        // 边缘类一般只做一遍
        if (type == Type::Sobel || type == Type::Laplacian
            || type == Type::Prewitt || type == Type::Roberts)
            break;
    }
    return current;
}

} // namespace FilterAlgorithm
