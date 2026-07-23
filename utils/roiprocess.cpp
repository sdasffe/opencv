/**
 * @file roiprocess.cpp
 * @brief ROI 蒙版工具：只改 ROI 并集内像素，外面保持原图
 *
 * 支持矩形、椭圆、旋转矩形三种 ROI 形状。
 */

#include "roiprocess.h"

#include <QtGlobal>
#include <algorithm>
#include <cmath>
#include <vector>

namespace RoiProcess {

namespace {

/** @brief 判断 ROI 列表中是否存在至少一个非空 ROI */
bool hasAnyRoi(const QList<RoiInfo> &rois)
{
    for (const RoiInfo &r : rois) {
        if (!r.isEmpty())
            return true; // 找到有效 ROI
    }
    return false; // 全部为空
}

/** @brief 将算法输出对齐到 srcBgr 的尺寸与通道类型，避免 copyTo 时尺寸不匹配 */
void alignProcessedToSrc(cv::Mat &processed, const cv::Mat &srcBgr)
{
    if (processed.size() != srcBgr.size())
        cv::resize(processed, processed, srcBgr.size(), 0, 0, cv::INTER_LINEAR); // 尺寸不一致则缩放
    if (processed.type() != srcBgr.type()) {
        if (srcBgr.channels() == 3 && processed.channels() == 1)
            cv::cvtColor(processed, processed, cv::COLOR_GRAY2BGR); // 灰度 → BGR
        else if (srcBgr.channels() == 1 && processed.channels() == 3)
            cv::cvtColor(processed, processed, cv::COLOR_BGR2GRAY); // BGR → 灰度
        else
            processed.convertTo(processed, srcBgr.type()); // 其他类型直接转换
    }
}

} // namespace

/** @brief 根据单个 ROI 生成 CV_8UC1 蒙版（255=处理区，0=保留原图） */
cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi)
{
    cv::Mat mask(size, CV_8UC1, cv::Scalar(255)); // 默认全图可处理
    if (roi.isEmpty())
        return mask; // 空 ROI 直接返回全 255

    mask.setTo(0); // 先清零，再填充 ROI 区域

    switch (roi.shape) {
    case RoiInfo::Shape::Rect: {
        const QRectF r = roi.rect.normalized(); // 规范化矩形坐标
        cv::Rect rr(qRound(r.x()), qRound(r.y()),
                    qRound(r.width()), qRound(r.height())); // Qt 坐标 → OpenCV Rect
        rr &= cv::Rect(0, 0, size.width, size.height); // 裁剪到图像边界内
        if (!rr.empty())
            mask(rr).setTo(255); // 矩形区域设为处理区
        break;
    }
    case RoiInfo::Shape::Ellipse: {
        const QRectF r = roi.rect.normalized(); // 椭圆外接矩形
        cv::Rect rr(qRound(r.x()), qRound(r.y()),
                    qRound(r.width()), qRound(r.height()));
        rr &= cv::Rect(0, 0, size.width, size.height); // 裁剪到图像边界内
        if (!rr.empty()) {
            cv::ellipse(mask,
                        cv::Point(rr.x + rr.width / 2, rr.y + rr.height / 2), // 椭圆中心
                        cv::Size(std::max(1, rr.width / 2), std::max(1, rr.height / 2)), // 半轴
                        0, 0, 360, cv::Scalar(255), cv::FILLED); // 填充椭圆
        }
        break;
    }
    case RoiInfo::Shape::RotatedRect: {
        const float rad = static_cast<float>(roi.angleDeg) * static_cast<float>(CV_PI) / 180.f; // 角度转弧度
        const float c = std::cos(rad);
        const float s = std::sin(rad);
        const float hw = static_cast<float>(roi.size.width()) / 2.f;  // 半宽
        const float hh = static_cast<float>(roi.size.height()) / 2.f; // 半高
        const cv::Point2f local[4] = {{-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}}; // 本地四角
        std::vector<cv::Point> poly;
        poly.reserve(4);
        for (const auto &p : local) {
            poly.emplace_back(cv::Point(
                cvRound(roi.center.x() + p.x * c - p.y * s), // 旋转 + 平移 X
                cvRound(roi.center.y() + p.x * s + p.y * c))); // 旋转 + 平移 Y
        }
        cv::fillConvexPoly(mask, poly, cv::Scalar(255)); // 填充旋转矩形
        break;
    }
    default:
        mask.setTo(255); // 未知形状回退为全图处理
        break;
    }
    return mask;
}

/** @brief 合并多个 ROI 蒙版为并集（OR） */
cv::Mat makeMask(const cv::Size &size, const QList<RoiInfo> &rois)
{
    if (!hasAnyRoi(rois))
        return cv::Mat(size, CV_8UC1, cv::Scalar(255)); // 无有效 ROI → 全图蒙版

    cv::Mat mask(size, CV_8UC1, cv::Scalar(0)); // 初始全 0
    for (const RoiInfo &roi : rois) {
        if (roi.isEmpty())
            continue; // 跳过空 ROI
        cv::Mat one = makeMask(size, roi); // 生成单个蒙版
        cv::bitwise_or(mask, one, mask);   // 并集合并
    }
    return mask;
}

/** @brief 单 ROI 便捷入口，内部包装为列表后调用主 apply */
cv::Mat apply(const cv::Mat &srcBgr,
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn)
{
    QList<RoiInfo> list;
    if (!roi.isEmpty())
        list.append(roi); // 非空 ROI 加入列表
    return apply(srcBgr, list, fn); // 委托多 ROI 版本
}

/** @brief ROI 局部处理：对整图执行算法，再用蒙版合成结果 */
cv::Mat apply(const cv::Mat &srcBgr,
              const QList<RoiInfo> &rois,
              const std::function<cv::Mat(const cv::Mat &)> &fn)
{
    if (srcBgr.empty() || !fn)
        return srcBgr.clone(); // 输入无效或无算法函数 → 返回原图副本

    cv::Mat processed = fn(srcBgr); // 对整图执行算法
    if (processed.empty())
        return srcBgr.clone(); // 算法输出为空 → 返回原图副本

    if (!hasAnyRoi(rois))
        return processed; // 无 ROI → 直接返回整图处理结果

    alignProcessedToSrc(processed, srcBgr); // 对齐尺寸与通道

    cv::Mat mask = makeMask(srcBgr.size(), rois); // 生成并集蒙版
    cv::Mat result = srcBgr.clone();              // 以原图为底
    processed.copyTo(result, mask);               // mask==255 处覆盖为算法结果
    return result;
}

} // namespace RoiProcess
