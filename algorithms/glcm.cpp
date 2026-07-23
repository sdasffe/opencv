/**
 * @file glcm.cpp
 * @brief GLCM 构建与 Haralick 特征提取
 *
 * 流程：BGR→灰度 → 量化 → 四方向累积共生矩阵 → 归一化 → 对比度/熵等
 */

#include "glcm.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace GlcmAlgorithm {

namespace {

/** 统一为 CV_8UC1 灰度，供量化与共生矩阵使用 */
cv::Mat toGrayU8(const cv::Mat &srcBgr)
{
    cv::Mat gray;
    if (srcBgr.channels() == 1)
        gray = srcBgr;
    else
        cv::cvtColor(srcBgr, gray, cv::COLOR_BGR2GRAY);
    if (gray.type() != CV_8UC1)
        gray.convertTo(gray, CV_8U);
    return gray;
}

/** 将 0~255 灰度映射到 [0, levels-1]，降低矩阵维度 */
cv::Mat quantize(const cv::Mat &gray, int levels)
{
    levels = std::clamp(levels, 2, 64);
    cv::Mat q(gray.size(), CV_8UC1);
    const double scale = (levels - 1) / 255.0;
    for (int y = 0; y < gray.rows; ++y) {
        const uchar *src = gray.ptr<uchar>(y);
        uchar *dst = q.ptr<uchar>(y);
        for (int x = 0; x < gray.cols; ++x)
            dst[x] = static_cast<uchar>(std::lround(src[x] * scale));
    }
    return q;
}

/** 按偏移 (dx,dy) 统计灰度对共现频次；对称矩阵 (i,j) 与 (j,i) 同时累加 */
void accumulateGlcm(const cv::Mat &q, int levels, int dx, int dy, cv::Mat &glcm)
{
    for (int y = 0; y < q.rows; ++y) {
        const int ny = y + dy;
        if (ny < 0 || ny >= q.rows)
            continue;
        const uchar *row = q.ptr<uchar>(y);
        const uchar *nrow = q.ptr<uchar>(ny);
        for (int x = 0; x < q.cols; ++x) {
            const int nx = x + dx;
            if (nx < 0 || nx >= q.cols)
                continue;
            const int i = row[x];
            const int j = nrow[nx];
            if (i < levels && j < levels) {
                glcm.at<float>(i, j) += 1.f;
                glcm.at<float>(j, i) += 1.f; // 对称
            }
        }
    }
}

/** 归一化共生矩阵后计算 Haralick 七项常用纹理量 */
Features featuresFromGlcm(const cv::Mat &glcm)
{
    Features f;
    const int n = glcm.rows;
    double sum = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            sum += glcm.at<float>(i, j);
    if (sum < 1e-12)
        return f;

    cv::Mat p;
    glcm.convertTo(p, CV_64F, 1.0 / sum);

    std::vector<double> px(n, 0), py(n, 0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            const double v = p.at<double>(i, j);
            px[i] += v;
            py[j] += v;
        }
    }

    double mux = 0, muy = 0;
    for (int i = 0; i < n; ++i) {
        mux += i * px[i];
        muy += i * py[i];
    }
    double sigx = 0, sigy = 0;
    for (int i = 0; i < n; ++i) {
        sigx += (i - mux) * (i - mux) * px[i];
        sigy += (i - muy) * (i - muy) * py[i];
    }
    sigx = std::sqrt(std::max(sigx, 0.0));
    sigy = std::sqrt(std::max(sigy, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            const double v = p.at<double>(i, j);
            const double d = i - j;
            f.contrast += d * d * v;
            f.homogeneity += v / (1.0 + d * d);
            f.dissimilarity += std::abs(d) * v;
            f.energy += v * v;
            if (v > 1e-12)
                f.entropy -= v * std::log(v);
            if (sigx > 1e-12 && sigy > 1e-12)
                f.correlation += ((i - mux) * (j - muy) * v) / (sigx * sigy);
        }
    }
    f.asmValue = f.energy;
    return f;
}

Features average(const Features &a, const Features &b, int countSoFar)
{
    // 在线平均：已有 countSoFar 个方向的结果 a，并入第 countSoFar+1 个方向 b
    Features o;
    const double n = countSoFar;
    const double inv = 1.0 / (n + 1.0);
    o.contrast = (a.contrast * n + b.contrast) * inv;
    o.correlation = (a.correlation * n + b.correlation) * inv;
    o.energy = (a.energy * n + b.energy) * inv;
    o.homogeneity = (a.homogeneity * n + b.homogeneity) * inv;
    o.entropy = (a.entropy * n + b.entropy) * inv;
    o.asmValue = (a.asmValue * n + b.asmValue) * inv;
    o.dissimilarity = (a.dissimilarity * n + b.dissimilarity) * inv;
    return o;
}

} // namespace

Features compute(const cv::Mat &srcBgr, int levels, int distance)
{
    Features out;
    if (srcBgr.empty())
        return out;

    levels = std::clamp(levels, 2, 64);
    distance = std::max(1, distance);

    const cv::Mat q = quantize(toGrayU8(srcBgr), levels);
    // 0°、45°、90°、135° 四个方向的 (dx, dy)
    const int dirs[4][2] = {
        { distance, 0 },
        { distance, distance },
        { 0, distance },
        { -distance, distance }
    };

    for (int d = 0; d < 4; ++d) {
        cv::Mat glcm = cv::Mat::zeros(levels, levels, CV_32F);
        accumulateGlcm(q, levels, dirs[d][0], dirs[d][1], glcm);
        const Features one = featuresFromGlcm(glcm);
        out = (d == 0) ? one : average(out, one, d);
    }
    return out;
}

} // namespace GlcmAlgorithm
