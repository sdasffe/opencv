# -*- coding: utf-8 -*-
"""生成各分支「函数调用先后顺序」流程图（独立 PNG，不写 docx）"""

from pathlib import Path
import math
from PIL import Image, ImageDraw, ImageFont

FIG = Path(__file__).resolve().parent / "figures"
FIG.mkdir(exist_ok=True)


def font(size, bold=False):
    candidates = [
        r"C:\Windows\Fonts\msyhbd.ttc" if bold else r"C:\Windows\Fonts\msyh.ttc",
        r"C:\Windows\Fonts\simhei.ttf",
        r"C:\Windows\Fonts\simsun.ttc",
    ]
    for p in candidates:
        if Path(p).exists():
            try:
                return ImageFont.truetype(p, size)
            except Exception:
                pass
    return ImageFont.load_default()


def rounded_rect(draw, xy, r, fill, outline=None, width=2):
    draw.rounded_rectangle(xy, radius=r, fill=fill, outline=outline, width=width)


def center_text(draw, box, text, fnt, fill=(30, 30, 30)):
    x0, y0, x1, y1 = box
    lines = text.split("\n")
    line_h = 18 if len(lines) > 1 else 0
    total_h = len(lines) * 18 if len(lines) > 1 else (draw.textbbox((0, 0), text, font=fnt)[3]
                                                       - draw.textbbox((0, 0), text, font=fnt)[1])
    cy = (y0 + y1 - total_h) / 2
    for i, line in enumerate(lines):
        bbox = draw.textbbox((0, 0), line, font=fnt)
        tw = bbox[2] - bbox[0]
        th = bbox[3] - bbox[1]
        draw.text(((x0 + x1 - tw) / 2, cy + i * (th + 4)), line, font=fnt, fill=fill)


def draw_arrow(draw, start, end, color=(60, 60, 60), width=2):
    draw.line([start, end], fill=color, width=width)
    x1, y1 = start
    x2, y2 = end
    angle = math.atan2(y2 - y1, x2 - x1)
    size = 9
    left = (x2 - size * math.cos(angle - 0.4), y2 - size * math.sin(angle - 0.4))
    right = (x2 - size * math.cos(angle + 0.4), y2 - size * math.sin(angle + 0.4))
    draw.polygon([end, left, right], fill=color)


def make_func_order(filename, title, steps, note=None,
                    fill="#E3F2FD", border="#1565C0",
                    box_w=420, box_h=44, gap=18, width=560):
    """
    steps: list[str]  自上而下的函数调用顺序
    画成：方框函数名，箭头表示先后
    """
    title_f, box_f, small = font(18, True), font(13, True), font(11)
    top = 70
    note_h = 50 if note else 16
    h = top + len(steps) * (box_h + gap) + note_h + 20
    img = Image.new("RGB", (width, h), (255, 255, 255))
    d = ImageDraw.Draw(img)

    d.text((24, 14), title, font=title_f, fill=(33, 33, 33))
    if note:
        d.text((24, 42), note, font=small, fill=(100, 100, 100))

    x = (width - box_w) // 2
    y = top if not note else top + 8

    for i, name in enumerate(steps):
        box = (x, y, x + box_w, y + box_h)
        rounded_rect(d, box, 8, fill, border, 2)
        center_text(d, box, name, box_f, (33, 33, 33))
        if i < len(steps) - 1:
            draw_arrow(d, (x + box_w // 2, y + box_h + 2),
                       (x + box_w // 2, y + box_h + gap - 2), (80, 80, 80), 2)
        y += box_h + gap

    path = FIG / filename
    img.save(path)
    print("OK", path)
    return path


def make_func_order_branch(filename, title, branches, note=None, width=1100):
    """
    branches: list[(branch_title, steps, fill, border)]
    多列并列的函数顺序（用于有分叉的路径）
    """
    title_f, phase_f, box_f, small = font(18, True), font(13, True), font(12, True), font(11)
    n = len(branches)
    max_steps = max(len(b[1]) for b in branches)
    col_w = (width - 40) // n
    box_w = min(280, col_w - 30)
    box_h = 40
    gap = 14
    top = 78 if note else 60
    h = top + 36 + max_steps * (box_h + gap) + 40
    img = Image.new("RGB", (width, h), (255, 255, 255))
    d = ImageDraw.Draw(img)

    d.text((24, 12), title, font=title_f, fill=(33, 33, 33))
    if note:
        d.text((24, 40), note, font=small, fill=(100, 100, 100))

    for bi, (btitle, steps, fill, border) in enumerate(branches):
        cx = 20 + bi * col_w + col_w // 2
        x = cx - box_w // 2
        # 分支标题
        tb = (x, top, x + box_w, top + 28)
        rounded_rect(d, tb, 6, fill, border, 2)
        center_text(d, tb, btitle, phase_f, (33, 33, 33))
        y = top + 40
        for i, name in enumerate(steps):
            box = (x, y, x + box_w, y + box_h)
            rounded_rect(d, box, 7, "#FFFFFF", border, 2)
            center_text(d, box, name, box_f, (40, 40, 40))
            if i < len(steps) - 1:
                draw_arrow(d, (cx, y + box_h + 1), (cx, y + box_h + gap - 1), (90, 90, 90), 2)
            y += box_h + gap

    path = FIG / filename
    img.save(path)
    print("OK", path)
    return path


def make_func_order_with_fork(filename, title, before, fork_label, yes_steps, no_steps, after,
                              note=None, width=900):
    """线性 → 分叉 →（可选）汇合后继续"""
    title_f, box_f, small = font(18, True), font(12, True), font(11)
    box_w, box_h, gap = 340, 40, 14
    fork_h = max(len(yes_steps), len(no_steps)) * (box_h + gap)
    top = 70
    h = top + len(before) * (box_h + gap) + 50 + fork_h + len(after) * (box_h + gap) + 40
    img = Image.new("RGB", (width, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    d.text((24, 14), title, font=title_f, fill=(33, 33, 33))
    if note:
        d.text((24, 42), note, font=small, fill=(100, 100, 100))

    cx = width // 2
    x = cx - box_w // 2
    y = top + (10 if note else 0)

    def draw_steps(x0, y0, steps, fill, border):
        yy = y0
        for i, name in enumerate(steps):
            box = (x0, yy, x0 + box_w, yy + box_h)
            rounded_rect(d, box, 7, fill, border, 2)
            center_text(d, box, name, box_f, (33, 33, 33))
            if i < len(steps) - 1:
                draw_arrow(d, (x0 + box_w // 2, yy + box_h + 1),
                           (x0 + box_w // 2, yy + box_h + gap - 1), (80, 80, 80), 2)
            yy += box_h + gap
        return yy

    y = draw_steps(x, y, before, "#E3F2FD", "#1565C0")

    # 菱形条件
    dy = y + 8
    diamond = [(cx, dy), (cx + 90, dy + 22), (cx, dy + 44), (cx - 90, dy + 22)]
    d.polygon(diamond, fill="#FFF9C4", outline="#F9A825")
    bbox = d.textbbox((0, 0), fork_label, font=small)
    d.text((cx - (bbox[2] - bbox[0]) / 2, dy + 14), fork_label, font=small, fill=(33, 33, 33))
    draw_arrow(d, (cx, y), (cx, dy), (80, 80, 80), 2)

    # 左右分支
    left_x = 40
    right_x = width - 40 - box_w
    left_w = min(box_w, 300)
    # 调整左右宽度
    left_x = 50
    right_x = width - 50 - left_w

    d.text((left_x + left_w // 2 - 10, dy + 50), "是", font=small, fill=(46, 125, 50))
    d.text((right_x + left_w // 2 - 10, dy + 50), "否", font=small, fill=(198, 40, 40))
    draw_arrow(d, (cx - 40, dy + 30), (left_x + left_w // 2, dy + 68), (46, 125, 50), 2)
    draw_arrow(d, (cx + 40, dy + 30), (right_x + left_w // 2, dy + 68), (198, 40, 40), 2)

    y_left = dy + 72
    y_right = dy + 72

    def draw_col(x0, y0, steps, fill, border):
        yy = y0
        for i, name in enumerate(steps):
            box = (x0, yy, x0 + left_w, yy + box_h)
            rounded_rect(d, box, 7, fill, border, 2)
            center_text(d, box, name, box_f, (33, 33, 33))
            if i < len(steps) - 1:
                draw_arrow(d, (x0 + left_w // 2, yy + box_h + 1),
                           (x0 + left_w // 2, yy + box_h + gap - 1), (80, 80, 80), 2)
            yy += box_h + gap
        return yy

    y_left = draw_col(left_x, y_left, yes_steps, "#C8E6C9", "#2E7D32")
    y_right = draw_col(right_x, y_right, no_steps, "#FFCDD2", "#C62828")

    if after:
        y = max(y_left, y_right) + 10
        # 汇合线
        mid_y = y
        d.line([(left_x + left_w // 2, y_left), (left_x + left_w // 2, mid_y),
                (right_x + left_w // 2, mid_y), (right_x + left_w // 2, y_right)],
               fill=(150, 150, 150), width=1)
        draw_arrow(d, (cx, mid_y), (cx, mid_y + 16), (80, 80, 80), 2)
        draw_steps(x, mid_y + 18, after, "#E8F5E9", "#2E7D32")

    path = FIG / filename
    img.save(path)
    print("OK", path)
    return path


def main():
    # 删掉之前误画的 UML 时序图
    for old in [
        "11_seq_boot.png", "12_seq_open_image.png", "13_seq_add_block.png",
        "14_seq_reprocess.png", "14b_seq_reprocess_triggers.png",
        "15_seq_roi.png", "16_seq_canvas.png", "17_seq_compare_save_chain.png",
    ]:
        p = FIG / old
        if p.exists():
            p.unlink()

    # ① 启动
    make_func_order(
        "11_func_boot.png",
        "函数顺序①：程序启动",
        [
            "Widget::Widget()",
            "ui->setupUi(this)",
            "adaptWindowToScreen()",
            "setupGraphicsView()",
            "setupDragDrop()",
            "setupBlockPanel()",
            "setupFolderBrowser()",
            "setupMenus()",
            "setupAlgoListIds()",
            "setupShortcuts()",
            "applyLanguage()",
            "connect(timer / processingFinished / requestReprocess)",
        ],
        note="构造函数内自上而下依次调用",
        fill="#E3F2FD", border="#1565C0",
    )

    # ② 打开图片（带分叉）
    make_func_order_with_fork(
        "12_func_open_image.png",
        "函数顺序②：打开图片",
        before=[
            "on_pushButton_clicked() / on_pushButton_2_clicked()",
            "clearFolderBrowser()（仅单张）",
            "loadImageFromPath()",
            "clearAllRoi()",
            "ImageProcessor::setOriginalImage()",
            "scene->addPixmap / fitInView",
        ],
        fork_label="链非空？",
        yes_steps=["onApplyProcessing()"],
        no_steps=["refreshDisplay()"],
        after=["updateInfoLabel()"],
        note="开图后有块则重算，无块则直接显示原图",
    )

    # ③ 拖入算法块
    make_func_order(
        "13_func_add_block.png",
        "函数顺序③：拖入算法块",
        [
            "eventFilter(Drop)",
            "createBlockByName()",
            "wireBinarizationOtsu()（仅二值化）",
            "addBlockToPanel()",
            "ImageProcessor::addBlock()",
            "emit requestReprocess",
            "onApplyProcessing()",
            "getCurrentRoiInfo() → setRoi() → reprocess()",
            "onProcessingFinished()",
            "refreshDisplay() → updatePixmapItem()",
        ],
        note="Drop 之后一路向下，最终汇入重算",
        fill="#FFF3E0", border="#EF6C00",
        box_w=460, width=600,
    )

    # ④ 重算核心（调参）
    make_func_order(
        "14_func_reprocess.png",
        "函数顺序④：重算核心（调参触发）",
        [
            "Block 控件变化",
            "emit paramsChanged / enabledChanged",
            "ImageProcessor::onBlockParamsChanged()",
            "emit requestReprocess",
            "Widget::onApplyProcessing()",
            "getCurrentRoiInfo()",
            "ImageProcessor::setRoi()",
            "ImageProcessor::reprocess()",
            "block->process()（逐启用块）",
            "emit processingFinished",
            "onProcessingFinished()",
            "refreshDisplay() → updatePixmapItem()",
            "updateInfoLabel()",
        ],
        note="所有重算路径最终都走 onApplyProcessing",
        fill="#F3E5F5", border="#7B1FA2",
        box_w=460, width=600,
    )

    # ④b 其它触发
    make_func_order_branch(
        "14b_func_reprocess_triggers.png",
        "函数顺序④b：其它触发 → 同一入口",
        [
            ("点「应用」", [
                "on_btnApply_clicked()",
                "onApplyProcessing()",
                "getCurrentRoiInfo()",
                "setRoi() + reprocess()",
                "onProcessingFinished()",
            ], "#E1BEE7", "#7B1FA2"),
            ("拖 ROI 停手", [
                "scene.changed",
                "m_roiUpdateTimer->start()",
                "timeout → onApplyProcessing()",
                "getCurrentRoiInfo()",
                "setRoi() + reprocess()",
            ], "#CE93D8", "#6A1B9A"),
            ("开图且有链", [
                "loadImageFromPath()",
                "onApplyProcessing()",
                "getCurrentRoiInfo()",
                "setRoi() + reprocess()",
                "onProcessingFinished()",
            ], "#BA68C8", "#4A148C"),
        ],
        note="三条支路函数不同，但都汇入 onApplyProcessing",
        width=1080,
    )

    # ⑤ ROI
    make_func_order_branch(
        "15_func_roi.png",
        "函数顺序⑤：ROI",
        [
            ("添加", [
                "on_pushButton_3_clicked()",
                "addRectItem() / addEllipseItem()",
                "  或 addRotatedRectItem()",
            ], "#C8E6C9", "#2E7D32"),
            ("删除", [
                "on_deltete_clicked()",
                "  或 keyPressEvent(Delete)",
                "clearAllRoi()",
            ], "#FFCDD2", "#C62828"),
            ("拖动改几何", [
                "ROI 图元几何变化",
                "scene.changed",
                "m_roiUpdateTimer 60ms",
                "onApplyProcessing()",
            ], "#BBDEFB", "#1565C0"),
        ],
        note="添加/删除改图元；拖动才会触发重算",
        width=1080,
    )

    # ⑥ 画布
    make_func_order_branch(
        "16_func_canvas.png",
        "函数顺序⑥：画布交互",
        [
            ("滚轮缩放", [
                "wheelEvent()",
                "graphicsView->scale()",
                "updateInfoLabel()",
            ], "#E3F2FD", "#1565C0"),
            ("平移", [
                "eventFilter(viewport)",
                "viewportPanEvent()",
                "改 ScrollBar 值",
            ], "#E8F5E9", "#2E7D32"),
            ("适应窗口", [
                "keyPressEvent(Ctrl+0)",
                "fitViewToImage()",
                "fitInView + 重置缩放",
            ], "#FFF3E0", "#EF6C00"),
        ],
        note="这三条都不进 reprocess",
        width=1080,
    )

    # ⑦ 对比 / 保存 / 链
    make_func_order_branch(
        "17_func_compare_save_chain.png",
        "函数顺序⑦：对比 / 保存 / 链管理",
        [
            ("对比（不重算）", [
                "on_btnCompare_pressed()",
                "m_showOriginal = true",
                "refreshDisplay()",
                "on_btnCompare_released()",
                "refreshDisplay()",
            ], "#FFF9C4", "#F9A825"),
            ("保存", [
                "on_btnSave_clicked()",
                "resultImage().save()",
            ], "#C8E6C9", "#2E7D32"),
            ("清空 / 导入导出", [
                "clearAllBlocks()",
                "  或 createBlockByName()",
                "JSON saveParams/loadParams",
                "（导入后）onApplyProcessing()",
            ], "#FFE0B2", "#EF6C00"),
        ],
        note="对比只切换显示；清空/导入可能触发重算",
        width=1080,
    )


if __name__ == "__main__":
    main()
