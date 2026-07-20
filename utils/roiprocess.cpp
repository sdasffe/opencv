/**
 * @file roiprocess.cpp
 * @brief ROI 蒙版工具：只改 ROI 内像素，外面保持原图
 *
 * 各算法块的典型用法：
 *   result = RoiProcess::apply(srcBgr, roi, [](const cv::Mat &m) {
 *       // 对整图 m 跑算法，返回处理后的图
 *       return myAlgorithm(m);
 *   });
 *
 * apply() 内部：
 *   1. 先对整图调用你的算法得到 processed
 *   2. 若 roi 为空 → 直接返回 processed（全图模式）
 *   3. 否则用 makeMask 生成 mask，把 processed 按 mask 贴回原图副本
 */

#include "roiprocess.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <vector>

namespace RoiProcess {

cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi)
{
    // 默认全 255 = 整图都处理
    cv::Mat mask(size, CV_8UC1, cv::Scalar(255));
    if (roi.isEmpty())
        return mask;

    // 有 ROI：先全黑，再把 ROI 形状涂成白
    mask.setTo(0);

    switch (roi.shape) {
    case RoiInfo::Shape::Rect: {
        const QRectF r = roi.rect.normalized();
        cv::Rect rr(qRound(r.x()), qRound(r.y()),
                    qRound(r.width()), qRound(r.height()));
        rr &= cv::Rect(0, 0, size.width, size.height);  // 裁到图像范围内
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
            // 用外接矩形中心和半轴画填充椭圆
            cv::ellipse(mask,
                        cv::Point(rr.x + rr.width / 2, rr.y + rr.height / 2),
                        cv::Size(std::max(1, rr.width / 2), std::max(1, rr.height / 2)),
                        0, 0, 360, cv::Scalar(255), cv::FILLED);
        }
        break;
    }
    case RoiInfo::Shape::RotatedRect: {
        // 本地四角 → 按 angleDeg 旋转 → 平移到 center → 填充多边形
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

    // 先整图跑算法（实现简单；ROI 外稍后用 mask 丢掉）
    cv::Mat processed = fn(srcBgr);
    if (processed.empty())
        return srcBgr.clone();

    // 无 ROI：整图结果直接用
    if (roi.isEmpty())
        return processed;

    // 尺寸/通道不一致时对齐到原图，避免 copyTo 失败导致“整图被改”
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
    processed.copyTo(result, mask);  // 只把 mask==255 的像素从 processed 拷过来
    return result;
}

} // namespace RoiProcess
