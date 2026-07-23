/**
 * @file morphology.cpp
 * @brief 形态学运算 —— 膨胀、腐蚀、开运算、闭运算
 *
 * 【在整条链路中的位置】
 *   MorphologyBlock::process → MorphologyAlgorithm::apply
 *
 * 【典型用途】
 *   - 膨胀：填补小孔、加粗前景
 *   - 腐蚀：去掉小噪点、细化前景
 *   - 开运算：先腐蚀后膨胀，去小亮斑
 *   - 闭运算：先膨胀后腐蚀，填小暗孔
 *
 * 【输入输出】
 *   输入/输出均为 BGR（三通道），结构元素对每通道独立作用
 */

#include "morphology.h"
#include <algorithm>

namespace MorphologyAlgorithm {

/**
 * @brief 将核尺寸转为合法奇数
 *
 * OpenCV 形态学/部分滤波要求核宽高为奇数（有明确中心像素）
 * 偶数则 +1；小于 1 则钳到 1
 */
static int oddSize(int v)
{
    v = std::max(1, v);
    if (v % 2 == 0) ++v;
    return v;
}

/**
 * @brief 执行形态学运算
 *
 * @param srcBgr    输入 BGR 图
 * @param op        运算类型
 * @param kx, ky    矩形结构元素宽、高（会 oddSize）
 * @param iterations 重复次数（等价于连续调用同一算子）
 * @return 处理后的 BGR 图
 *
 * cv::Point(-1,-1) 表示锚点在核中心（默认）
 * 结构元素类型 MORPH_RECT：轴对齐矩形核
 */
cv::Mat apply(const cv::Mat &srcBgr, Op op, int kx, int ky, int iterations)
{
    if (srcBgr.empty()) return srcBgr.clone();
    iterations = std::max(1, iterations);

    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT, cv::Size(oddSize(kx), oddSize(ky)));

    cv::Mat dst;
    switch (op) {
    case Op::Dilate:
        // 膨胀：邻域内有前景则中心变前景
        cv::dilate(srcBgr, dst, kernel, cv::Point(-1, -1), iterations);
        break;
    case Op::Erode:
        // 腐蚀：邻域内全为背景才中心变背景
        cv::erode(srcBgr, dst, kernel, cv::Point(-1, -1), iterations);
        break;
    case Op::Open:
        // 开 = 腐蚀 + 膨胀，去小亮点
        cv::morphologyEx(srcBgr, dst, cv::MORPH_OPEN, kernel,
                         cv::Point(-1, -1), iterations);
        break;
    case Op::Close:
        // 闭 = 膨胀 + 腐蚀，填小暗点
        cv::morphologyEx(srcBgr, dst, cv::MORPH_CLOSE, kernel,
                         cv::Point(-1, -1), iterations);
        break;
    case Op::TopHat:
        //顶帽=输入-开运算
        cv::morphologyEx(srcBgr,dst,cv::MORPH_TOPHAT,kernel,
                         cv::Point(-1,-1),iterations);
        break;
    case Op::ButtonHat:
        //底帽=闭运算-输入
        cv::morphologyEx(srcBgr,dst,cv::MORPH_BLACKHAT,kernel,
                         cv::Point(-1,-1),iterations);
        break;
    case Op::MorphologicalGradient:
        // 形态学梯度 = 膨胀 − 腐蚀，突出物体轮廓
        cv::morphologyEx(srcBgr,dst,cv::MORPH_GRADIENT,kernel,
                         cv::Point(-1,-1),iterations);
    }
    return dst;
}

} // namespace MorphologyAlgorithm
