/**
 * @file roiprocess.cpp
 * @brief ROI 蒙版工具：只改 ROI 并集内像素，外面保持原图
 *
 * 【在整条链路中的位置】
 *   FilterBlock / BinarizationBlock / MorphologyBlock 等 → apply(src, rois, λ)
 *   与 binarization.cpp 中 apply*Roi* 旧接口并存；新块统一走本模块 + 整图算法
 *
 * 【形状支持】
 *   矩形、椭圆（外接 rect）、旋转矩形（与 Qt 图元同一套角度/顶点计算）
 */

#include "roiprocess.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <vector>

namespace RoiProcess {

namespace {

/** 列表中是否存在至少一个非空 ROI（用于区分「全图」与「局部」） */
bool hasAnyRoi(const QList<RoiInfo> &rois)
{
    for (const RoiInfo &r : rois) {
        if (!r.isEmpty())
            return true;
    }
    return false;
}

/**
 * 算法输出可能与输入尺寸/通道不一致（如边缘检测出单通道），
 * 在 copyTo 合成前对齐到 srcBgr，避免 mask 与 result 尺寸不匹配
 */
void alignProcessedToSrc(cv::Mat &processed, const cv::Mat &srcBgr)
{
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
}

} // namespace

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
        // 本地四角 → 顺时针旋转 → 平移，与 Widget / binarization 旋转 ROI 一致
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

cv::Mat makeMask(const cv::Size &size, const QList<RoiInfo> &rois)
{
    if (!hasAnyRoi(rois))
        return cv::Mat(size, CV_8UC1, cv::Scalar(255));

    cv::Mat mask(size, CV_8UC1, cv::Scalar(0));
    for (const RoiInfo &roi : rois) {
        if (roi.isEmpty())
            continue;
        cv::Mat one = makeMask(size, roi);
        cv::bitwise_or(mask, one, mask);
    }
    return mask;
}

cv::Mat apply(const cv::Mat &srcBgr,
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn)
{
    QList<RoiInfo> list;
    if (!roi.isEmpty())
        list.append(roi);
    return apply(srcBgr, list, fn);
}

cv::Mat apply(const cv::Mat &srcBgr,
              const QList<RoiInfo> &rois,
              const std::function<cv::Mat(const cv::Mat &)> &fn)
{
    if (srcBgr.empty() || !fn)
        return srcBgr.clone();

    cv::Mat processed = fn(srcBgr);
    if (processed.empty())
        return srcBgr.clone();

    if (!hasAnyRoi(rois))
        return processed;

    alignProcessedToSrc(processed, srcBgr);

    cv::Mat mask = makeMask(srcBgr.size(), rois);
    cv::Mat result = srcBgr.clone();
    processed.copyTo(result, mask); // mask==255 处覆盖，其余保留原图
    return result;
}

} // namespace RoiProcess
