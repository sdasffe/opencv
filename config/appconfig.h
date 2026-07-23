#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>

/**
 * @brief 全局配置常量
 *
 * 集中管理所有默认值、尺寸限制、样式常量等，避免魔法数字散落在各处。
 *
 * 注意：含中文的 const char* 均为 UTF-8，转 QString 必须用 QString::fromUtf8，
 * 不要用 fromLatin1（否则会出现 å¾·å° 这类乱码）。
 */
namespace AppConfig {

/** UTF-8 C 字符串 → QString（中文配置专用） */
inline QString trUtf8(const char *utf8)
{
    return QString::fromUtf8(utf8);
}

// ========== 应用信息 ==========
const char* const APP_NAME_ZH = "图像处理工具";
const char* const APP_VERSION = "1.0.0";

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
constexpr int BLOCK_FIELD_LABEL_WIDTH = 56; // 参数行标签宽度（容纳「下限值」等）
constexpr int BLOCK_SPIN_WIDTH = 96;       // SpinBox 宽度（数字 + 上下箭头）

// ========== 主题色（与 styles/app.qss 保持一致） ==========
const char* const COLOR_PRIMARY = "#0F766E";      // 主色（青绿）
const char* const COLOR_PRIMARY_HOVER = "#0D9488";
const char* const COLOR_ACCENT = "#2563EB";       // 强调色（蓝）
const char* const COLOR_DANGER = "#DC2626";       // 危险色（红）
const char* const COLOR_BORDER = "#E2E8F0";       // 边框色
const char* const COLOR_BG_APP = "#E8ECF1";       // 窗口背景
const char* const COLOR_BG_PANEL = "#FFFFFF";     // 侧栏面板
const char* const COLOR_BG_CANVAS = "#1A1D23";    // 画布背景
const char* const COLOR_BG_LIGHT = "#F8FAFC";     // 浅底

// ========== 支持的图像格式 ==========
const char* const IMAGE_FILE_FILTER =
    "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;所有文件 (*.*)";

/** 处理链导入/导出 */
const char* const CHAIN_FILE_FILTER =
    "处理链 (*.json);;所有文件 (*.*)";

// ========== 处理块名称（与UI列表对应） ==========
const char* const BLOCK_NAME_BINARIZATION = "二值化处理";
const char* const BLOCK_NAME_MORPHOLOGY = "形态学处理";
const char* const BLOCK_NAME_FILTER = "滤波处理";
const char* const BLOCK_NAME_GRAYTRANSFORM = "灰度变换";
const char* const BLOCK_NAME_PSEUDOCOLOR = "伪彩色处理";
const char* const BLOCK_NAME_GLCM = "灰度共生矩阵";

// ========== 拖放 MIME ==========
/** 已生成算法块在右侧面板内拖动换序（与左侧工具箱的 text 拖入区分） */
const char* const MIME_BLOCK_REORDER = "application/x-opencv-block-reorder";
/** 处理块复制到剪贴板（JSON 参数） */
const char* const MIME_BLOCK_CLIPBOARD = "application/x-opencv-block-clipboard";

} // namespace AppConfig

#endif // APPCONFIG_H
