#ifndef ROIPROCESS_H
#define ROIPROCESS_H

#include "../blocksdk/blocksdk_global.h"
#include <functional>
#include <QList>
#include "../roi/roiinfo.h"
#include "opencv2/opencv.hpp"

namespace RoiProcess {

BLOCKSDK_EXPORT cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi);
BLOCKSDK_EXPORT cv::Mat makeMask(const cv::Size &size, const QList<RoiInfo> &rois);

BLOCKSDK_EXPORT cv::Mat apply(const cv::Mat &srcBgr,
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

BLOCKSDK_EXPORT cv::Mat apply(const cv::Mat &srcBgr,
              const QList<RoiInfo> &rois,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

} // namespace RoiProcess

#endif // ROIPROCESS_H
