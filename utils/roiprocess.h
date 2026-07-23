#ifndef ROIPROCESS_H
#define ROIPROCESS_H

#include <functional>
#include <QList>
#include "../roi/roiinfo.h"
#include "opencv2/opencv.hpp"

/**
 * @file roiprocess.h
 * @brief ROI 蒙版与局部处理 —— 算法块与 ROI 图元之间的桥梁
 *
 * 多 ROI 做并集（OR）；空列表 = 全图处理。
 */
namespace RoiProcess {

cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi);              // 单 ROI → CV_8UC1 蒙版（255=处理区）；空 ROI 返回全 255
cv::Mat makeMask(const cv::Size &size, const QList<RoiInfo> &rois);      // 多 ROI 并集蒙版；列表全空则等同全图（全 255）

cv::Mat apply(const cv::Mat &srcBgr,                                       // 单 ROI 便捷重载，内部转为列表后调用 apply(list)
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

cv::Mat apply(const cv::Mat &srcBgr,                                       // ROI 局部处理主入口：ROI 内为 fn 结果，ROI 外保持原像素
              const QList<RoiInfo> &rois,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

} // namespace RoiProcess

#endif // ROIPROCESS_H
