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
 * 【在整条链路中的位置】
 *   各 *Block::process 内：RoiProcess::apply(当前图, session.rois, λ算法)
 *   → 算法对整图运算，再用 mask 只保留 ROI 内结果，ROI 外保持原像素
 *
 * 多 ROI：makeMask/apply 对列表做并集（OR）；空列表 = 全图处理。
 */

namespace RoiProcess {

/** 单 ROI → CV_8UC1 蒙版（255=处理区）；空 ROI 返回全 255 */
cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi);

/** 多 ROI 并集蒙版；列表全空则等同全图（全 255） */
cv::Mat makeMask(const cv::Size &size, const QList<RoiInfo> &rois);

/**
 * 单 ROI 便捷重载，内部转为列表后调用 apply(list)
 * @param fn 整图算法，如 [](const cv::Mat &m){ return FilterAlgorithm::apply(...); }
 */
cv::Mat apply(const cv::Mat &srcBgr,
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

/**
 * ROI 局部处理主入口
 * @param srcBgr 当前流水线 BGR/灰度图
 * @param rois   来自 ImageSession，与画布图元同步
 * @param fn     对整图执行的算法；输出尺寸/通道会自动对齐 srcBgr
 * @return 合成图：ROI 内为 fn 结果，ROI 外为 srcBgr 原像素
 */
cv::Mat apply(const cv::Mat &srcBgr,
              const QList<RoiInfo> &rois,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

} // namespace RoiProcess

#endif // ROIPROCESS_H
