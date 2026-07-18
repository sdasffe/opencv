#include "graytransform.h"

#include <cmath>

namespace GrayTransformAlgorithm {

static cv::Mat ensureBgr(const cv::Mat &src)
{
    if (src.channels() == 1) {
        cv::Mat bgr;
        cv::cvtColor(src, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    return src;
}

cv::Mat apply(const cv::Mat &srcBgr, Type type,
              int brightness, int contrast, double gamma)
{
    if (srcBgr.empty()) return srcBgr.clone();

    switch (type) {
    case Type::ToGray: {
        cv::Mat gray, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(gray, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    case Type::BrightContrast: {
        // alpha = contrast/100, beta = brightness
        const double alpha = std::max(0.0, contrast / 100.0);
        cv::Mat dst;
        srcBgr.convertTo(dst, -1, alpha, brightness);
        return dst;
    }
    case Type::Invert: {
        cv::Mat dst;
        cv::bitwise_not(srcBgr, dst);
        return dst;
    }
    case Type::Log: {
        cv::Mat gray, f, out, u8, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        gray.convertTo(f, CV_32F, 1.0 / 255.0);
        cv::log(f + 1.0f, out);
        double maxv;
        cv::minMaxLoc(out, nullptr, &maxv);
        if (maxv > 1e-6)
            out = out * (255.0 / maxv);
        out.convertTo(u8, CV_8U);
        cv::cvtColor(u8, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    case Type::Gamma: {
        gamma = std::max(0.05, gamma);
        cv::Mat f, out;
        srcBgr.convertTo(f, CV_32F, 1.0 / 255.0);
        cv::pow(f, gamma, out);
        out.convertTo(out, CV_8U, 255.0);
        return ensureBgr(out);
    }
    case Type::Equalize: {
        cv::Mat gray, eq, bgr;
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, eq);
        cv::cvtColor(eq, bgr, cv::COLOR_GRAY2BGR);
        return bgr;
    }
    case Type::Normalize: {
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
