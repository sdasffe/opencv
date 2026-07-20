#ifndef ROIPROCESS_H
#define ROIPROCESS_H

#include <functional>
#include "../roi/roiinfo.h"
#include "opencv2/opencv.hpp"

/**
 * @file roiprocess.h
 * @brief ROI 蒙版与局部处理 —— 算法块与 ROI 图元之间的桥梁
 *
 * 【在整条链路中的位置】
 *   ResizableRectItem 等图元 → Widget 收集几何 → RoiInfo
 *   → 各 Block::process(input, roi) → RoiProcess::apply(...)
 *   → 仅 ROI 内像素被算法修改，外部保持原图
 *
 * 【设计思路】
 *   算法函数 fn 始终接收「整图」Mat，实现简单；
 *   apply() 负责生成 mask，用 copyTo(mask) 把结果贴回原图副本。
 *   这样每个算法不必各自实现 ROI 裁剪逻辑。
 *
 * 【RoiInfo 来源】
 *   roi/roiinfo.h 定义形状（None/Rect/Ellipse/RotatedRect）与几何参数；
 *   isEmpty() 为 true 时等价于全图处理（mask 全 255）。
 *
 * 【实现文件】
 *   utils/roiprocess.cpp —— makeMask / apply 的具体逻辑
 */

namespace RoiProcess {

/**
 * @brief 生成与图像同尺寸的二值蒙版
 *
 * 谁调用：apply() 内部；也可单独用于调试 mask 可视化
 * 何时调用：roi 非空且需要局部处理时
 *
 * @param size 原图宽高（与 srcBgr.size() 一致）
 * @param roi  当前 ROI 描述；isEmpty() 时返回全 255
 * @return CV_8UC1：255 = 该像素参与处理，0 = 保持原图
 *
 * 各形状：
 *   Rect        —— 轴对齐矩形，rect 字段
 *   Ellipse     —— 外接矩形内填充椭圆
 *   RotatedRect —— center + size + angleDeg，手动算四角填多边形
 */
cv::Mat makeMask(const cv::Size &size, const RoiInfo &roi);

/**
 * @brief 在 ROI 内应用算法，ROI 外保留原像素
 *
 * 谁调用：几乎所有 Block::process() 中的 lambda 包装
 * 何时调用：每次 ImageProcessor 重算且 roi 可能非空时
 *
 * @param srcBgr 输入 BGR 图（与流水线约定一致）
 * @param roi    ROI 信息；空则 fn 结果整图返回
 * @param fn     算法回调：Mat(BGR) → Mat(处理结果)，尺寸/通道应与原图一致
 * @return 合成后的 BGR 图
 *
 * 内部流程（详见 roiprocess.cpp）：
 *   1. processed = fn(srcBgr)     // 整图跑算法
 *   2. 若 roi 空 → 直接 return processed
 *   3. mask = makeMask(...)
 *   4. result = srcBgr.clone()
 *   5. processed.copyTo(result, mask)  // 仅 mask==255 处覆盖
 */
cv::Mat apply(const cv::Mat &srcBgr,
              const RoiInfo &roi,
              const std::function<cv::Mat(const cv::Mat &)> &fn);

} // namespace RoiProcess

#endif // ROIPROCESS_H
