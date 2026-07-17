#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

/**
 * @brief 全局配置常量
 *
 * 集中管理所有默认值、尺寸限制、样式常量等，避免魔法数字散落在各处。
 */
namespace AppConfig {

// ========== 图像显示 ==========
constexpr double MIN_SCALE_FACTOR = 0.1;   // 最小缩放比例
constexpr double MAX_SCALE_FACTOR = 5.0;   // 最大缩放比例
constexpr double SCROLL_SCALE_STEP = 1.25; // 滚轮缩放步长

// ========== ROI 默认值 ==========
constexpr int DEFAULT_ROI_SIZE = 100;      // ROI 默认尺寸（像素）

// ========== 二值化默认值 ==========
constexpr int DEFAULT_BINARY_LOWER = 127;    // 默认下限阈值
constexpr int DEFAULT_BINARY_UPPER = 255;  // 默认上限阈值

// ========== 处理块样式 ==========
constexpr int BLOCK_MIN_HEIGHT = 160;      // 处理块最小高度
constexpr int BLOCK_LAYOUT_MARGIN = 8;     // 处理块面板边距
constexpr int BLOCK_LAYOUT_SPACING = 8;    // 处理块之间间距

// ========== 主题色 ==========
const char* const COLOR_PRIMARY = "#4CAF50";   // 主色（绿）
const char* const COLOR_ACCENT = "#2196F3";    // 强调色（蓝）
const char* const COLOR_DANGER = "#ff5252";    // 危险色（红）
const char* const COLOR_BORDER = "#ddd";       // 边框色
const char* const COLOR_BG_LIGHT = "#f5f5f5";  // 浅灰背景

// ========== 支持的图像格式 ==========
const char* const IMAGE_FILE_FILTER =
    "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;所有文件 (*.*)";

// ========== 处理块名称（与UI列表对应） ==========
const char* const BLOCK_NAME_BINARIZATION = "二值化处理";
const char* const BLOCK_NAME_MORPHOLOGY = "形态学处理";
const char* const BLOCK_NAME_FILTER = "滤波处理";
const char* const BLOCK_NAME_GRAYTRANSFORM = "灰度变换";
const char* const BLOCK_NAME_PSEUDOCOLOR = "伪彩色处理";

} // namespace AppConfig

#endif // APPCONFIG_H
