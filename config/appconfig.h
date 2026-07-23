#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

/**
 * @file appconfig.h
 * @brief 全局配置常量
 *
 * 含中文的 const char* 均为 UTF-8，转 QString 须用 QString::fromUtf8。
 */
namespace AppConfig {

inline QString trUtf8(const char *utf8) // UTF-8 C 字符串 → QString（中文配置专用）
{
    return QString::fromUtf8(utf8);
}

// ========== 应用信息 ==========
const char* const APP_NAME_ZH = "图像处理工具"; // 应用中文名称
const char* const APP_VERSION = "1.0.0";        // 应用版本号

// ========== 图像显示 ==========
constexpr double MIN_SCALE_FACTOR = 0.1;    // 画布最小缩放比例
constexpr double MAX_SCALE_FACTOR = 5.0;    // 画布最大缩放比例
constexpr double SCROLL_SCALE_STEP = 1.25;  // 滚轮缩放步长倍数

// ========== ROI 默认值 ==========
constexpr int DEFAULT_ROI_SIZE = 100; // ROI 默认尺寸（像素）

// ========== 二值化默认值 ==========
constexpr int DEFAULT_BINARY_LOWER = 127; // 二值化默认下限阈值
constexpr int DEFAULT_BINARY_UPPER = 255; // 二值化默认上限阈值

// ========== 处理块样式 ==========
constexpr int BLOCK_MIN_HEIGHT = 160;       // 处理块面板最小高度
constexpr int BLOCK_LAYOUT_MARGIN = 8;      // 处理块面板内边距
constexpr int BLOCK_LAYOUT_SPACING = 8;     // 处理块之间的间距
constexpr int BLOCK_FIELD_LABEL_WIDTH = 56; // 参数行标签宽度（容纳「下限值」等中文）
constexpr int BLOCK_SPIN_WIDTH = 96;        // SpinBox 控件宽度

// ========== 主题色（与 styles/theme_*.qss 保持一致） ==========
const char* const COLOR_PRIMARY = "#0F766E";      // 主色（青绿）
const char* const COLOR_PRIMARY_HOVER = "#0D9488"; // 主色悬停态
const char* const COLOR_ACCENT = "#2563EB";       // 强调色（蓝）
const char* const COLOR_DANGER = "#DC2626";       // 危险/删除色（红）
const char* const COLOR_BORDER = "#E2E8F0";       // 通用边框色
const char* const COLOR_BG_APP = "#E8ECF1";       // 窗口背景色
const char* const COLOR_BG_PANEL = "#FFFFFF";     // 侧栏面板背景色
const char* const COLOR_BG_CANVAS = "#1A1D23";    // 画布区域背景色
const char* const COLOR_BG_LIGHT = "#F8FAFC";     // 浅色底背景

// ========== 支持的图像格式 ==========
const char* const IMAGE_FILE_FILTER =
    "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;所有文件 (*.*)"; // 打开/保存图片对话框过滤器

const char* const CHAIN_FILE_FILTER =
    "处理链 (*.json);;所有文件 (*.*)"; // 处理链导入/导出对话框过滤器

// ========== 处理块名称（与 UI 列表对应） ==========
const char* const BLOCK_NAME_BINARIZATION = "二值化处理";   // 二值化块显示名
const char* const BLOCK_NAME_MORPHOLOGY = "形态学处理";     // 形态学块显示名
const char* const BLOCK_NAME_FILTER = "滤波处理";           // 滤波块显示名
const char* const BLOCK_NAME_GRAYTRANSFORM = "灰度变换";    // 灰度变换块显示名
const char* const BLOCK_NAME_PSEUDOCOLOR = "伪彩色处理";    // 伪彩色块显示名
const char* const BLOCK_NAME_GLCM = "灰度共生矩阵";         // GLCM 块显示名

// ========== 拖放 MIME ==========
const char* const MIME_BLOCK_REORDER = "application/x-opencv-block-reorder";   // 右侧面板内算法块换序拖放类型
const char* const MIME_BLOCK_CLIPBOARD = "application/x-opencv-block-clipboard"; // 处理块复制到剪贴板的 MIME 类型

} // namespace AppConfig

#endif // APPCONFIG_H
