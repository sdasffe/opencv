#include "roiprocess.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <vector>

namespace RoiProcess {

cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi)
{
    cv::Mat mask(size, CV_8UC1, cv::Scalar(255));
    if (roi.isEmpty())
        return mask;

    mask.setTo(0);

    switch (roi.shape) {
    case RoiInfo::Shape::Rect: {
        const QRectF r = roi.rect.normalized();
        cv::Rect rr(qRound(r.x()), qRound(r.y()),
                    qRound(r.width()), qRound(r.height()));
        rr &= cv::Rect(0, 0, size.width, size.height);
        if (!rr.empty())
            mask(rr).setTo(255);
        break;
    }
    case RoiInfo::Shape::Ellipse: {
        const QRectF r = roi.rect.normalized();
        cv::Rect rr(qRound(r.x()), qRound(r.y()),
                    qRound(r.width()), qRound(r.height()));
        rr &= cv::Rect(0, 0, size.width, size.height);
        if (!rr.empty()) {
            cv::ellipse(mask,
                        cv::Point(rr.x + rr.width / 2, rr.y + rr.height / 2),
                        cv::Size(std::max(1, rr.width / 2), std::max(1, rr.height / 2)),
                        0, 0, 360, cv::Scalar(255), cv::FILLED);
        }
        break;
    }
    case RoiInfo::Shape::RotatedRect: {
        const float rad = static_cast<float>(roi.angleDeg) * static_cast<float>(CV_PI) / 180.f;
        const float c = std::cos(rad);
        const float s = std::sin(rad);
        const float hw = static_cast<float>(roi.size.width()) / 2.f;
        const float hh = static_cast<float>(roi.size.height()) / 2.f;
        const cv::Point2f local[4] = {{-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}};
        std::vector<cv::Point> poly;
        poly.reserve(4);
        for (const auto &p : local) {
            poly.emplace_back(cv::Point(
                cvRound(roi.center.x() + p.x * c - p.y * s),
                cvRound(roi.center.y() + p.x * s + p.y * c)));
        }
        cv::fillConvexPoly(mask, poly, cv::Scalar(255));
        break;
    }
    default:
        mask.setTo(255);
        break;
    }
    return mask;
}

cv::Mat apply(const cv::Mat &srcBgr,
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn)
{
    if (srcBgr.empty() || !fn)
        return srcBgr.clone();

    cv::Mat processed = fn(srcBgr);
    if (processed.empty())
        return srcBgr.clone();

    // 无 ROI：整图结果
    if (roi.isEmpty())
        return processed;

    // 尺寸/通道不一致时，尽量对齐到原图，避免 copyTo 失败导致“整图被改”
    if (processed.size() != srcBgr.size())
        cv::resize(processed, processed, srcBgr.size(), 0, 0, cv::INTER_LINEAR);
    if (processed.type() != srcBgr.type()) {
        if (srcBgr.channels() == 3 && processed.channels() == 1)
            cv::cvtColor(processed, processed, cv::COLOR_GRAY2BGR);
        else if (srcBgr.channels() == 1 && processed.channels() == 3)
            cv::cvtColor(processed, processed, cv::COLOR_BGR2GRAY);
        else
            processed.convertTo(processed, srcBgr.type());
    }

    cv::Mat mask = makeMask(srcBgr.size(), roi);
    cv::Mat result = srcBgr.clone(); // ROI 外：与输入完全一致
    processed.copyTo(result, mask);
    return result;
}

} // namespace RoiProcess
