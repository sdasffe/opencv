# -*- coding: utf-8 -*-
from pathlib import Path

data = {
    "Widget": [
        ("图像处理工具", "Image Processing Tool"),
        ("文件", "File"),
        ("ROI", "ROI"),
        ("设置", "Settings"),
        ("应用", "Apply"),
        ("对比", "Compare"),
        ("保存", "Save"),
        ("耗时", "Time"),
        ("算法", "Algorithms"),
        ("图像处理工具箱", "Processing Toolbox"),
        ("导出", "Export"),
        ("导入", "Import"),
        ("清空", "Clear"),
        ("拖入算法", "Drag algorithms here"),
        ("打开图片", "Open Image"),
        ("打开文件夹", "Open Folder"),
        ("退出", "Exit"),
        ("语言", "Language"),
        ("中文", "Chinese"),
        ("English", "English"),
        ("关于", "About"),
        ("帮助", "Help"),
        ("矩形", "Rectangle"),
        ("圆形", "Ellipse"),
        ("旋转矩形", "Rotated Rect"),
        ("添加", "Add"),
        ("删除", "Delete"),
        ("二值化处理", "Binarization"),
        ("形态学处理", "Morphology"),
        ("滤波处理", "Filter"),
        ("灰度变换", "Gray Transform"),
        ("伪彩色处理", "Pseudo Color"),
        ("灰度共生矩阵", "GLCM"),
        ("文件夹图片（点击切换）", "Folder images (click to switch)"),
        ("点击缩略图切换当前图片", "Click a thumbnail to switch image"),
        ("按当前 ROI 与处理链重新计算", "Recompute with current ROI and chain"),
        ("按住查看原图，松开恢复结果", "Hold to view original, release for result"),
        ("保存处理结果", "Save result image"),
        ("导出当前处理链为 JSON", "Export chain as JSON"),
        ("从 JSON 导入处理链", "Import chain from JSON"),
        ("清空图像处理工具箱", "Clear processing toolbox"),
        ("图片信息", "Image info"),
        ("ROI 选区", "ROI selection"),
        ("选择图片", "Select Image"),
        ("选择图片文件夹", "Select Image Folder"),
        ("提示", "Notice"),
        ("错误", "Error"),
        ("完成", "Done"),
        ("请先打开一张图片", "Please open an image first"),
        ("请先打开图片", "Please open an image first"),
        ("无法加载图片，文件可能已损坏", "Failed to load image, file may be corrupted"),
        ("无法加载图片", "Failed to load image"),
        ("该文件夹下没有找到图片文件", "No images found in this folder"),
        ("保存失败", "Save failed"),
        ("当前没有处理块可导出", "No blocks to export"),
        ("导出处理链", "Export Chain"),
        ("导入处理链", "Import Chain"),
        ("已导出 %1 个处理块", "Exported %1 block(s)"),
        ("无法写入文件：%1", "Cannot write file: %1"),
        ("无法读取文件：%1", "Cannot read file: %1"),
        ("JSON 无效：%1", "Invalid JSON: %1"),
        ("文件中没有处理块", "No blocks in file"),
        ("导入将替换当前处理链，是否继续？", "Import will replace the current chain. Continue?"),
        ("未能识别文件中的任何处理块", "No recognizable blocks in file"),
        ("已导入 %1 / %2 个处理块（部分名称无法识别）", "Imported %1 / %2 block(s) (some names unknown)"),
        ("未识别的算法：%1", "Unknown algorithm: %1"),
        (
            "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;所有文件 (*.*)",
            "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;All Files (*.*)",
        ),
        ("处理链 (*.json);;所有文件 (*.*)", "Processing Chain (*.json);;All Files (*.*)"),
        (
            "%1\n版本 %2\n\n基于 Qt Widgets + OpenCV 的图像处理演示程序。",
            "%1\nVersion %2\n\nQt Widgets + OpenCV image processing demo.",
        ),
        ("图像处理工具 — %1  (%2 × %3)", "Image Processing Tool — %1  (%2 × %3)"),
        ("无法加载图片：%1", "Failed to load image: %1"),
        ("缺少 blockListContainer，请检查 widget.ui", "Missing blockListContainer, check widget.ui"),
        ("界面错误", "UI Error"),
        ("没有可保存的结果图", "No result image to save"),
        ("保存结果", "Save Result"),
        ("是否删除", "Confirm Delete"),
        ("你确定要执行这个操作吗？", "Are you sure you want to do this?"),
        ("剪贴板中没有有效的处理块", "Clipboard has no valid processing block"),
        ("粘贴", "Paste"),
        (
            "快捷键\n"
            "• Delete — 删除 ROI\n"
            "• Ctrl+0 — 画布适应窗口\n"
            "• 滚轮 — 缩放画布（以鼠标为中心）\n"
            "• 中键 / 左键空白处拖动 — 平移画布\n"
            "• 按住「对比」— 查看原图，松开恢复结果\n"
            "\n"
            "处理链\n"
            "• 从左侧拖入算法到右侧工具箱\n"
            "• 拖动块标题 — 调整处理顺序\n"
            "• 右键处理块 — 复制 / 粘贴 / 删除\n"
            "• 右键空白处 — 粘贴到链尾\n"
            "• 导入 / 导出 — 处理链 JSON 文件",
            "Shortcuts\n"
            "• Delete — Delete ROI\n"
            "• Ctrl+0 — Fit view to window\n"
            "• Mouse wheel — Zoom (anchor under cursor)\n"
            "• Middle click / left-drag on empty area — Pan\n"
            "• Hold Compare — View original, release for result\n"
            "\n"
            "Processing chain\n"
            "• Drag algorithms from the left into the toolbox\n"
            "• Drag block title — Reorder\n"
            "• Right-click a block — Copy / Paste / Delete\n"
            "• Right-click empty area — Paste at end\n"
            "• Import / Export — Chain JSON file",
        ),
    ],
    "BaseBlock": [
        ("拖动可调整处理顺序", "Drag to reorder"),
        ("开", "On"),
        ("启用此处理块", "Enable this block"),
        ("删除此处理块", "Delete this block"),
        ("启用%1", "Enable %1"),
        ("右键可复制、粘贴或删除", "Right-click to copy, paste, or delete"),
        ("复制", "Copy"),
        ("粘贴", "Paste"),
        ("删除", "Delete"),
    ],
    "BinarizationBlock": [
        ("二值化处理", "Binarization"),
        ("下限值", "Lower"),
        ("上限值", "Upper"),
        ("自动阈值 (Otsu)", "Auto threshold (Otsu)"),
    ],
    "MorphologyBlock": [
        ("形态学处理", "Morphology"),
        ("膨胀", "Dilate"),
        ("腐蚀", "Erode"),
        ("开运算", "Open"),
        ("闭运算", "Close"),
        ("顶帽", "Top Hat"),
        ("底帽", "Black Hat"),
        ("形态学梯度", "Gradient"),
        ("核 X", "Kernel X"),
        ("核 Y", "Kernel Y"),
        ("次数", "Iterations"),
    ],
    "FilterBlock": [
        ("滤波处理", "Filter"),
        ("均值滤波", "Mean"),
        ("高斯滤波", "Gaussian"),
        ("中值滤波", "Median"),
        ("Sobel", "Sobel"),
        ("Laplacian", "Laplacian"),
        ("Prewitt", "Prewitt"),
        ("Roberts", "Roberts"),
        ("核 X", "Kernel X"),
        ("核 Y", "Kernel Y"),
        ("次数", "Iterations"),
    ],
    "GrayTransformBlock": [
        ("灰度变换", "Gray Transform"),
        ("转灰度", "To Gray"),
        ("亮度/对比度", "Brightness/Contrast"),
        ("反转", "Invert"),
        ("对数变换", "Log"),
        ("伽马变换", "Gamma"),
        ("直方图均衡", "Equalize Hist"),
        ("归一化", "Normalize"),
        ("亮度", "Brightness"),
        ("对比度", "Contrast"),
        ("伽马", "Gamma"),
    ],
    "PseudoColorBlock": [
        ("伪彩色处理", "Pseudo Color"),
        ("喷射色", "Jet"),
        ("热力色", "Hot"),
        ("冷色系", "Cool"),
        ("彩虹色", "Rainbow"),
        ("海洋色", "Ocean"),
        ("夏日色", "Summer"),
        ("冬日色", "Winter"),
        ("秋日色", "Autumn"),
        ("骨白色", "Bone"),
        ("粉红色", "Pink"),
    ],
    "GlcmBlock": [
        ("灰度共生矩阵", "GLCM"),
        ("查看：对比度", "View: Contrast"),
        ("查看：相关性", "View: Correlation"),
        ("查看：能量", "View: Energy"),
        ("查看：均匀性", "View: Homogeneity"),
        ("查看：熵", "View: Entropy"),
        ("查看：相异性", "View: Dissimilarity"),
        ("量化级", "Levels"),
        ("距离", "Distance"),
        ("对比度", "Contrast"),
        ("相关性", "Correlation"),
        ("能量", "Energy"),
        ("均匀性", "Homogeneity"),
        ("熵", "Entropy"),
        ("相异性", "Dissimilarity"),
    ],
}


def esc(s: str) -> str:
    return (
        s.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


lines = [
    '<?xml version="1.0" encoding="utf-8"?>',
    "<!DOCTYPE TS>",
    '<TS version="2.1" language="en_US" sourcelanguage="zh_CN">',
]
for ctx, pairs in data.items():
    lines.append("<context>")
    lines.append(f"<name>{ctx}</name>")
    for src, trn in pairs:
        lines.append("<message>")
        lines.append(f"<source>{esc(src)}</source>")
        lines.append(f"<translation>{esc(trn)}</translation>")
        lines.append("</message>")
    lines.append("</context>")
lines.append("</TS>")

out = Path(__file__).with_name("opencv_en.ts")
out.write_text("\n".join(lines), encoding="utf-8")
print("wrote", out)
