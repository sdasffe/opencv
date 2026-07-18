#ifndef ROIPROCESS_H
#define ROIPROCESS_H

#include <functional>
#include "../roi/roiinfo.h"
#include "opencv2/opencv.hpp"

/**
 * @brief 按 RoiInfo 生成 mask，并仅在 ROI 内应用算法
 */
namespace RoiProcess {

/** 生成与图像同尺寸的 mask：255=处理区；roi 为空则全 255 */
cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi);

/**
 * @brief 对整图跑算法，再按 mask 贴回（ROI 外保持原图）
 * @param srcBgr 输入 BGR
 * @param fn 接收整图 BGR，返回处理后的 BGR（尺寸/通道需一致）
 */
cv::Mat apply(const cv::Mat &srcBgr,
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

} // namespace RoiProcess

#endif // ROIPROCESS_H
