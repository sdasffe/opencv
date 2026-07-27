/**
 * @file morphology.cpp
 * @brief 形态学运算 —— 膨胀、腐蚀、开/闭、顶帽/底帽、梯度
 *
 * MorphologyBlock::process → MorphologyAlgorithm::apply
 * 输入/输出均为 BGR；结构元素对每通道独立作用。
 */

#include "morphology.h"
#include <algorithm>

namespace MorphologyAlgorithm {

/** @brief 核尺寸改为合法奇数（OpenCV 需明确中心像素） */
static int oddSize(int v)
{
    v = std::max(1, v);                                              // 至少 1
    if (v % 2 == 0) ++v;                                             // 偶数则 +1
    return v;
}

/**
 * @brief 执行形态学运算
 * @param srcBgr 输入 BGR；@param op 运算类型；@param kx,ky 核宽高；@param iterations 次数
 */
cv::Mat apply(const cv::Mat &srcBgr, Op op, int kx, int ky, int iterations)
{
    if (srcBgr.empty()) return srcBgr.clone();                       // 空图直通
    iterations = std::max(1, iterations);                            // 至少迭代 1 次

    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT, cv::Size(oddSize(kx), oddSize(ky)));         // 轴对齐矩形结构元

    cv::Mat dst;
    switch (op) {
    case Op::Dilate:
        cv::dilate(srcBgr, dst, kernel, cv::Point(-1, -1), iterations); // 膨胀：邻域有前景则中心前景
        break;
    case Op::Erode:
        cv::erode(srcBgr, dst, kernel, cv::Point(-1, -1), iterations);  // 腐蚀：邻域全背景才变背景
        break;
    case Op::Open:
        cv::morphologyEx(srcBgr, dst, cv::MORPH_OPEN, kernel,
                         cv::Point(-1, -1), iterations);             // 开 = 腐蚀+膨胀，去小亮斑
        break;
    case Op::Close:
        cv::morphologyEx(srcBgr, dst, cv::MORPH_CLOSE, kernel,
                         cv::Point(-1, -1), iterations);             // 闭 = 膨胀+腐蚀，填小暗孔
        break;
    case Op::TopHat:
        cv::morphologyEx(srcBgr, dst, cv::MORPH_TOPHAT, kernel,
                         cv::Point(-1, -1), iterations);             // 顶帽 = 输入 − 开运算
        break;
    case Op::ButtonHat:
        cv::morphologyEx(srcBgr, dst, cv::MORPH_BLACKHAT, kernel,
                         cv::Point(-1, -1), iterations);             // 底帽 = 闭运算 − 输入
        break;
    case Op::MorphologicalGradient:
        cv::morphologyEx(srcBgr, dst, cv::MORPH_GRADIENT, kernel,
                         cv::Point(-1, -1), iterations);             // 梯度 = 膨胀 − 腐蚀，突出轮廓
        break;
    }
    return dst;
}

} // namespace MorphologyAlgorithm
