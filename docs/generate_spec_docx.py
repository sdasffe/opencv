# -*- coding: utf-8 -*-
"""生成《图像处理工具》IPD 概念与计划说明（朴素语气 + IPD 框架）"""

from pathlib import Path
import shutil
import math
from PIL import Image, ImageDraw, ImageFont
from docx import Document
from docx.shared import Pt, Cm, RGBColor, Inches
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_LINE_SPACING
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

ROOT = Path(__file__).resolve().parent
FIG = ROOT / "figures"
FIG.mkdir(exist_ok=True)
OUT = ROOT / "ImageProcessingTool-Concept-Design-Spec.docx"
# 图像处理工具-IPD概念与计划说明.docx
OUT_CN = ROOT / (
    "".join(chr(c) for c in [
        0x56FE, 0x50CF, 0x5904, 0x7406, 0x5DE5, 0x5177, 0x2D,
        0x0049, 0x0050, 0x0044,
        0x6982, 0x5FF5, 0x4E0E, 0x8BA1, 0x5212, 0x8BF4, 0x660E,
    ]) + ".docx"
)


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
    bbox = draw.textbbox((0, 0), text, font=fnt)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    draw.text(((x0 + x1 - tw) / 2, (y0 + y1 - th) / 2), text, font=fnt, fill=fill)


def draw_arrow(draw, start, end, color=(60, 60, 60), width=2):
    draw.line([start, end], fill=color, width=width)
    x1, y1 = start
    x2, y2 = end
    angle = math.atan2(y2 - y1, x2 - x1)
    size = 10
    left = (x2 - size * math.cos(angle - 0.4), y2 - size * math.sin(angle - 0.4))
    right = (x2 - size * math.cos(angle + 0.4), y2 - size * math.sin(angle + 0.4))
    draw.polygon([end, left, right], fill=color)


# ========== IPD 总框架图 ==========
def make_ipd_framework():
    w, h = 1180, 720
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(26, True), font(15, True), font(13)

    d.text((30, 18), "IPD 文档框架（领导要求的写法）", font=title_f, fill=(33, 33, 33))

    # CDCP
    rounded_rect(d, (40, 70, 560, 340), 12, (232, 245, 233), (46, 125, 50), 3)
    d.text((60, 90), "概念阶段 CDCP", font=box_f, fill=(27, 94, 32))
    d.text((60, 125), "先想清楚：做什么、为谁做、做成啥", font=small, fill=(60, 60, 60))
    items_l = [
        "1 产品概念（背景/愿景/用户/目标/范围）",
        "2 功能与非功能需求",
        "3 用例模型",
        "4 系统架构 4+1 视图",
        "5 界面与交互设计",
    ]
    for i, t in enumerate(items_l):
        d.text((70, 165 + i * 30), t, font=small, fill=(40, 40, 40))

    # PDCP
    rounded_rect(d, (620, 70, 1140, 340), 12, (227, 242, 253), (21, 101, 192), 3)
    d.text((640, 90), "计划阶段 PDCP", font=box_f, fill=(13, 71, 161))
    d.text((640, 125), "再想清楚：怎么排期、怎么验收、风险在哪", font=small, fill=(60, 60, 60))
    items_r = [
        "6 模块划分与扩展套路",
        "7 数据与关键流程",
        "8 项目计划与版本路线",
        "9 质量、风险与验收",
        "10 加分能力与后续规划",
    ]
    for i, t in enumerate(items_r):
        d.text((650, 165 + i * 30), t, font=small, fill=(40, 40, 40))

    draw_arrow(d, (560, 200), (620, 200), (46, 125, 50), 4)

    # bottom explanation
    rounded_rect(d, (40, 380, 1140, 680), 12, (255, 248, 225), (245, 124, 0), 3)
    d.text((60, 400), "新人怎么理解 IPD（说人话）", font=box_f, fill=(230, 81, 0))
    lines = [
        "IPD = 先做正确的事，再把事做正确。",
        "概念评审（CDCP）：对齐「值不值得做、范围锁不锁得住」。通过后，才进入计划。",
        "计划评审（PDCP）：对齐「四周怎么交付、验收看什么、风险怎么挡」。",
        "本文语气尽量朴素，但章节仍按 IPD 两段式组织，方便领导按检查点审阅。",
        "本项目定位：入职第一个月 Qt + OpenCV 练手 Demo，不是工业产线系统。",
        "写作顺序建议：概念 → 需求/用例/架构/UI → 计划/验收 → 加分项。",
    ]
    for i, line in enumerate(lines):
        d.text((60, 450 + i * 32), line, font=small, fill=(50, 50, 50))

    path = FIG / "00_ipd_framework.png"
    img.save(path)
    return path


def make_usecase():
    w, h = 1100, 720
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, actor_f, uc_f, note_f = font(24, True), font(16, True), font(14), font(12)

    d.text((30, 20), "用例图：使用者能做什么", font=title_f, fill=(33, 33, 33))
    rounded_rect(d, (260, 70, 1040, 660), 12, (248, 252, 248), (76, 175, 80), 3)
    d.text((500, 85), "图像处理工具（本 Demo）", font=actor_f, fill=(46, 125, 50))

    ax, ay = 90, 320
    d.ellipse([ax - 20, ay - 60, ax + 20, ay - 20], outline=(33, 33, 33), width=3)
    d.line([(ax, ay - 20), (ax, ay + 35)], fill=(33, 33, 33), width=3)
    d.line([(ax - 30, ay), (ax + 30, ay)], fill=(33, 33, 33), width=3)
    d.line([(ax, ay + 35), (ax - 22, ay + 80)], fill=(33, 33, 33), width=3)
    d.line([(ax, ay + 35), (ax + 22, ay + 80)], fill=(33, 33, 33), width=3)
    d.text((ax - 30, ay + 90), "使用者", font=actor_f, fill=(33, 33, 33))
    d.text((ax - 55, ay + 118), "（学习者/验证者）", font=note_f, fill=(100, 100, 100))

    usecases = [
        (480, 150, "打开并浏览图片"),
        (780, 150, "画 ROI（矩形/椭圆）"),
        (480, 260, "拖入算法处理块"),
        (780, 260, "调节参数看效果"),
        (480, 370, "开关 / 删除处理块"),
        (780, 370, "查看处理耗时"),
        (480, 480, "原图/结果对比切换"),
        (780, 480, "保存处理结果"),
        (630, 580, "一键应用常用模板"),
    ]
    for x, y, name in usecases:
        box = (x - 120, y - 26, x + 120, y + 26)
        rounded_rect(d, box, 26, (232, 245, 233), (76, 175, 80), 2)
        center_text(d, box, name, uc_f, (27, 94, 32))
        draw_arrow(d, (120, 340), (x - 120, y), (150, 150, 150), 2)

    path = FIG / "01_usecase.png"
    img.save(path)
    return path


def make_logical():
    w, h = 1100, 700
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, layer_f, item_f = font(24, True), font(16, True), font(13)
    d.text((30, 18), "逻辑视图：分层架构（4+1 之一）", font=title_f, fill=(33, 33, 33))

    layers = [
        ("表现层", "#E3F2FD", "#1565C0", "主窗口、拖放列表、ROI 交互、界面刷新"),
        ("应用层", "#E8F5E9", "#2E7D32", "ImageProcessor：处理链调度、ROI、耗时、信号"),
        ("领域层", "#FFF3E0", "#EF6C00", "BaseBlock 策略基类 + 各算法块 + algorithms"),
        ("基础设施层", "#F3E5F5", "#7B1FA2", "ImageConverter、TimeMeasurer、AppConfig"),
    ]
    y = 70
    for name, fill, border, desc in layers:
        rounded_rect(d, (60, y, 1040, y + 120), 10, fill, border, 3)
        d.text((80, y + 20), name, font=layer_f, fill=border)
        d.text((80, y + 60), desc, font=item_f, fill=(50, 50, 50))
        y += 140
    d.text((60, 650), "一句话：界面管交互，应用层串流程，领域层做算法，基础设施提供工具。", font=item_f, fill=(70, 70, 70))
    path = FIG / "02_logical.png"
    img.save(path)
    return path


def make_development():
    w, h = 1000, 560
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(22, True), font(14, True), font(13)
    d.text((30, 15), "开发视图：目录与依赖（4+1 之一）", font=title_f, fill=(33, 33, 33))

    items = [
        (500, 80, "main.cpp 入口", "#CFD8DC"),
        (500, 170, "core/ 主窗口 + 调度", "#90CAF9"),
        (180, 300, "roi/\n矩形/椭圆 ROI", "#A5D6A7"),
        (500, 300, "blocks/\n处理块 UI", "#FFE082"),
        (820, 300, "algorithms/\n纯 OpenCV", "#FFAB91"),
        (320, 450, "utils/ 转换、计时", "#CE93D8"),
        (680, 450, "config/ 常量", "#B0BEC5"),
    ]
    for x, y, text, color in items:
        rounded_rect(d, (x - 120, y - 35, x + 120, y + 45), 8, color, (66, 66, 66), 2)
        lines = text.split("\n")
        for i, line in enumerate(lines):
            bbox = d.textbbox((0, 0), line, font=box_f if i == 0 else small)
            tw = bbox[2] - bbox[0]
            d.text((x - tw / 2, y - 18 + i * 22), line, font=box_f if i == 0 else small, fill=(33, 33, 33))

    draw_arrow(d, (500, 115), (500, 135))
    draw_arrow(d, (430, 215), (250, 265))
    draw_arrow(d, (500, 215), (500, 265))
    draw_arrow(d, (570, 215), (750, 265))
    draw_arrow(d, (450, 345), (360, 415))
    draw_arrow(d, (580, 345), (650, 415))
    draw_arrow(d, (580, 340), (740, 340))
    path = FIG / "03_development.png"
    img.save(path)
    return path


def make_process():
    w, h = 1100, 420
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(22, True), font(14, True), font(12)
    d.text((30, 15), "进程视图：单进程事件驱动（4+1 之一）", font=title_f, fill=(33, 33, 33))

    steps = [
        (120, 160, "用户操作\n打开/拖放/调参"),
        (340, 160, "Widget\n触发重算"),
        (560, 160, "ImageProcessor\n按序跑处理链"),
        (780, 160, "各 Block\nprocess()"),
        (980, 160, "刷新预览\n更新耗时"),
    ]
    for x, y, text in steps:
        rounded_rect(d, (x - 85, y - 50, x + 85, y + 50), 10, (227, 242, 253), (25, 118, 210), 2)
        for i, line in enumerate(text.split("\n")):
            bbox = d.textbbox((0, 0), line, font=box_f)
            tw = bbox[2] - bbox[0]
            d.text((x - tw / 2, y - 18 + i * 22), line, font=box_f, fill=(33, 33, 33))
    for i in range(4):
        x1 = 120 + i * 220 + 85
        x2 = 120 + (i + 1) * 220 - 85
        draw_arrow(d, (x1, 160), (x2, 160), (25, 118, 210), 3)
    d.text((60, 280), "运行形态：Windows 单进程桌面程序，Qt 主线程事件循环驱动。", font=small, fill=(80, 80, 80))
    d.text((60, 310), "调参 / 拖 ROI / 开关块时，都会重新走一遍处理链。", font=small, fill=(80, 80, 80))
    d.text((60, 340), "后续若大图卡顿，可再考虑工作线程异步（本月先保证正确可用）。", font=small, fill=(80, 80, 80))
    path = FIG / "04_process.png"
    img.save(path)
    return path


def make_physical():
    w, h = 1000, 420
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(22, True), font(15, True), font(13)
    d.text((30, 15), "物理视图：开发与运行环境（4+1 之一）", font=title_f, fill=(33, 33, 33))

    nodes = [
        (180, 180, "开发机", ["Windows 10/11", "Qt Creator + MSVC", "Qt 6 + OpenCV 4", "本工程源码"]),
        (500, 180, "运行工件", ["opencv.exe", "Qt 运行时 DLL", "opencv_world DLL", "本地图片"]),
        (820, 180, "输入输出", ["jpg/png/bmp…", "结果图导出", "可选工程/ROI 文件"]),
    ]
    for x, y, title, items in nodes:
        rounded_rect(d, (x - 130, y - 70, x + 130, y + 150), 10, (232, 245, 233), (56, 142, 60), 2)
        d.text((x - 110, y - 55), title, font=box_f, fill=(27, 94, 32))
        for i, it in enumerate(items):
            d.text((x - 110, y - 10 + i * 32), "• " + it, font=small, fill=(40, 40, 40))
    draw_arrow(d, (310, 180), (370, 180))
    draw_arrow(d, (630, 180), (690, 180))
    d.text((60, 370), "部署：单机桌面应用，无需服务器。", font=small, fill=(80, 80, 80))
    path = FIG / "05_physical.png"
    img.save(path)
    return path


def make_scenario():
    w, h = 1100, 380
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, small = font(22, True), font(12)
    d.text((30, 12), "场景视图（+1）：拖入二值化并调参预览", font=title_f, fill=(33, 33, 33))

    actors = ["使用者", "Widget", "ImageProcessor", "BinarizationBlock", "algorithms"]
    xs = [100, 300, 520, 760, 980]
    for x, name in zip(xs, actors):
        d.ellipse([x - 16, 70, x + 16, 102], outline=(33, 33, 33), width=2)
        d.line([(x, 102), (x, 340)], fill=(180, 180, 180), width=1)
        bbox = d.textbbox((0, 0), name, font=small)
        d.text((x - (bbox[2] - bbox[0]) / 2, 48), name, font=small, fill=(33, 33, 33))

    msgs = [
        (0, 1, 130, "拖入「二值化」"),
        (1, 2, 165, "addBlock"),
        (0, 3, 200, "调节阈值 / Otsu"),
        (3, 2, 235, "paramsChanged"),
        (2, 3, 270, "process(input, roi)"),
        (3, 4, 305, "调用 OpenCV"),
        (2, 1, 340, "刷新预览 + 耗时"),
    ]
    for a, b, y, text in msgs:
        x1, x2 = xs[a], xs[b]
        draw_arrow(d, (x1 + 20 if x2 > x1 else x1 - 20, y), (x2 - 20 if x2 > x1 else x2 + 20, y), (66, 66, 66), 2)
        mid = (x1 + x2) / 2
        bbox = d.textbbox((0, 0), text, font=small)
        d.text((mid - (bbox[2] - bbox[0]) / 2, y - 16), text, font=small, fill=(40, 40, 40))

    path = FIG / "06_scenario.png"
    img.save(path)
    return path


def make_ui():
    w, h = 1200, 700
    img = Image.new("RGB", (w, h), (245, 247, 250))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(22, True), font(14, True), font(12)
    d.text((30, 15), "主界面布局（三栏）", font=title_f, fill=(33, 33, 33))
    rounded_rect(d, (40, 60, 1160, 660), 8, (255, 255, 255), (120, 120, 120), 2)

    rounded_rect(d, (55, 80, 720, 640), 6, (250, 250, 250), (180, 180, 180), 2)
    d.text((70, 95), "左侧：图片预览区", font=box_f, fill=(33, 33, 33))
    rounded_rect(d, (70, 130, 700, 175), 4, (76, 175, 80), None, 0)
    d.text((85, 142), "打开图片 | ROI | 添加/删除 | 原图/结果 | 保存", font=small, fill=(255, 255, 255))
    rounded_rect(d, (70, 190, 700, 560), 4, (236, 239, 241), (160, 160, 160), 1)
    center_text(d, (70, 190, 700, 560), "图片 + ROI（可拖、可缩放）", font(16), (100, 100, 100))
    d.text((70, 575), "状态栏：分辨率 / 缩放 / 总耗时", font=small, fill=(90, 90, 90))

    rounded_rect(d, (735, 80, 920, 640), 6, (255, 255, 255), (180, 180, 180), 2)
    d.text((750, 95), "中栏：算法列表", font=box_f, fill=(33, 33, 33))
    for i, a in enumerate(["二值化", "形态学", "滤波", "灰度变换", "伪彩色"]):
        y = 140 + i * 55
        rounded_rect(d, (755, y, 900, y + 42), 6, (232, 245, 233), (76, 175, 80), 1)
        center_text(d, (755, y, 900, y + 42), a, small, (27, 94, 32))
    rounded_rect(d, (755, 470, 900, 540), 6, (255, 243, 224), (239, 108, 0), 1)
    center_text(d, (755, 470, 900, 540), "常用模板\n去噪→二值化", small, (230, 81, 0))

    rounded_rect(d, (935, 80, 1145, 640), 6, (255, 255, 255), (180, 180, 180), 2)
    d.text((950, 95), "右栏：处理链", font=box_f, fill=(33, 33, 33))
    for i, (t, sub) in enumerate([("二值化处理", "阈值 / Otsu / 使能"), ("形态学", "核大小 / 次数")]):
        y = 140 + i * 130
        rounded_rect(d, (950, y, 1130, y + 110), 8, (255, 255, 255), (33, 150, 243), 2)
        d.text((965, y + 12), t, font=box_f, fill=(21, 101, 192))
        d.text((965, y + 42), sub, font=small, fill=(80, 80, 80))
        d.text((965, y + 72), "[使能]          [删除]", font=small, fill=(120, 120, 120))
    d.text((950, 420), "从上到下 = 执行顺序", font=small, fill=(100, 100, 100))

    path = FIG / "07_ui.png"
    img.save(path)
    return path


# ---------- Word helpers ----------
def set_run_font(run, size=11, bold=False, color=None, name="微软雅黑"):
    run.font.size = Pt(size)
    run.font.bold = bold
    run.font.name = name
    r = run._element
    rPr = r.get_or_add_rPr()
    rFonts = rPr.get_or_add_rFonts()
    rFonts.set(qn("w:eastAsia"), name)
    if color:
        run.font.color.rgb = RGBColor(*color)


def set_cell_shading(cell, hex_color):
    tc = cell._tc
    tcPr = tc.get_or_add_tcPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:fill"), hex_color)
    shd.set(qn("w:val"), "clear")
    tcPr.append(shd)


def add_heading(doc, text, level=1):
    p = doc.add_heading(text, level=level)
    for run in p.runs:
        set_run_font(run, size=16 if level == 1 else 13, bold=True, name="微软雅黑")
    return p


def add_para(doc, text, size=11, bold=False, center=False, space_after=6):
    p = doc.add_paragraph()
    if center:
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_after = Pt(space_after)
    p.paragraph_format.line_spacing_rule = WD_LINE_SPACING.ONE_POINT_FIVE
    run = p.add_run(text)
    set_run_font(run, size=size, bold=bold)
    return p


def add_bullets(doc, items):
    for it in items:
        p = doc.add_paragraph(style="List Bullet")
        run = p.add_run(it)
        set_run_font(run, size=11)
        p.paragraph_format.space_after = Pt(2)


def add_table(doc, headers, rows, col_widths=None):
    table = doc.add_table(rows=1 + len(rows), cols=len(headers))
    table.style = "Table Grid"
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    for i, h in enumerate(headers):
        cell = table.rows[0].cells[i]
        cell.text = ""
        p = cell.paragraphs[0]
        run = p.add_run(h)
        set_run_font(run, size=10, bold=True, color=(255, 255, 255))
        set_cell_shading(cell, "4CAF50")
    for r_i, row in enumerate(rows):
        for c_i, val in enumerate(row):
            cell = table.rows[r_i + 1].cells[c_i]
            cell.text = ""
            p = cell.paragraphs[0]
            run = p.add_run(str(val))
            set_run_font(run, size=10)
            if r_i % 2 == 1:
                set_cell_shading(cell, "F5F5F5")
    if col_widths:
        for row in table.rows:
            for i, w in enumerate(col_widths):
                row.cells[i].width = Cm(w)
    doc.add_paragraph()
    return table


def add_caption(doc, text):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = p.add_run(text)
    set_run_font(run, size=10, color=(90, 90, 90))
    p.paragraph_format.space_after = Pt(10)


def add_picture(doc, path, width_in=6.2):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.add_run().add_picture(str(path), width=Inches(width_in))


def build_doc(figures):
    doc = Document()
    for section in doc.sections:
        section.top_margin = Cm(2.2)
        section.bottom_margin = Cm(2.2)
        section.left_margin = Cm(2.4)
        section.right_margin = Cm(2.4)

    # 封面
    for _ in range(2):
        doc.add_paragraph()
    add_para(doc, "图像处理工具", size=28, bold=True, center=True, space_after=10)
    add_para(doc, "IPD 产品概念与项目计划说明", size=18, bold=True, center=True, space_after=8)
    add_para(doc, "（概念决策 CDCP + 计划决策 PDCP）", size=12, center=True, space_after=18)
    add_para(doc, "定位：入职第一个月 Qt + OpenCV 练手 Demo", size=11, center=True)
    add_para(doc, "技术栈：C++17 / Qt 6 Widgets / OpenCV 4.x / qmake", size=11, center=True)
    add_para(doc, "工程名：opencv　　文档版本：V1.0　　日期：2026-07-18", size=11, center=True, space_after=16)
    add_para(
        doc,
        "写法说明：按领导要求采用 IPD 两段式（先概念、后计划）；\n"
        "正文尽量说人话，方便新人写、领导审。",
        size=10,
        center=True,
    )
    doc.add_page_break()

    # 0 框架
    add_heading(doc, "0 IPD 文档框架（先看这张图）", 1)
    add_para(doc, "整份文档按 IPD 的「概念 → 计划」组织。概念阶段回答「做什么」，计划阶段回答「怎么做完、怎么验收」。")
    add_picture(doc, figures["ipd"], 6.4)
    add_caption(doc, "图 0-1 IPD 文档框架")

    add_table(
        doc,
        ["IPD 检查点", "本章对应", "领导主要看什么"],
        [
            ["CDCP 概念决策", "第 1～5 章", "值不值得做、范围清不清、架构能不能撑住"],
            ["PDCP 计划决策", "第 6～10 章", "四周怎么排、验收看什么、风险有没有挡"],
        ],
        [3.5, 4, 6],
    )

    # ========== CDCP ==========
    add_heading(doc, "【IPD-CDCP】概念阶段：把事情想清楚", 1)
    add_para(doc, "概念阶段目标：对齐产品定位、用户价值、功能边界和架构基线。通过后再进入计划与开发。")

    add_heading(doc, "1 产品概念", 1)
    add_heading(doc, "1.1 项目背景", 2)
    add_para(
        doc,
        "图像算法学习/验证时，常见做法是写脚本或跑示例，缺少「看得见、拖得动、能串联」的一体化体验。"
        "本项目做一个基于 Qt Widgets + OpenCV 的可视化小工具，作为入职第一个月的综合练手 Demo，"
        "同时练习 Graphics View、拖放、自定义 Item、信号槽和分层扩展。",
    )

    add_heading(doc, "1.2 产品愿景与一句话概念", 2)
    add_para(doc, "愿景：做成可扩展的图像算法验证小工作台——加一个算法，尽量只加「算法 + 参数面板 + 列表注册」。")
    add_para(
        doc,
        "一句话概念：基于 Qt + OpenCV 的可视化图像处理工具，支持 ROI、拖拽处理链、实时调参预览和耗时显示。",
        bold=True,
    )

    add_heading(doc, "1.3 目标用户与价值", 2)
    add_table(
        doc,
        ["用户角色", "诉求", "本 Demo 提供的价值"],
        [
            ["图像处理学习者", "边调参边看图，建立算法直觉", "降低上手门槛，即时反馈"],
            ["算法验证人员", "快速试阈值、核大小、次数等组合", "缩短试错周期，可看耗时"],
            ["新人自己（作者）", "练 Qt/OpenCV，并能按 IPD 讲清楚", "完整小闭环 + 可演示材料"],
        ],
        [3, 5, 5.5],
    )

    add_heading(doc, "1.4 产品目标", 2)
    add_table(
        doc,
        ["目标", "描述", "成功判据"],
        [
            ["可视化调参", "参数经 UI 调节并实时预览", "调参后尽快看到效果变化"],
            ["ROI 标注", "矩形/椭圆可拖拽缩放删除", "ROI 操作顺畅，局部处理正确"],
            ["链式处理", "多个算法块按序串联", "拖入后从上到下依次生效"],
            ["可扩展", "新算法按固定套路加入", "尽量不改动既有块逻辑"],
            ["性能可感知", "显示处理链总耗时", "演示时可口头对比不同参数"],
        ],
        [2.5, 5.5, 5.5],
    )

    add_heading(doc, "1.5 范围边界（CDCP 要锁住的）", 2)
    add_para(doc, "范围内：", bold=True)
    add_bullets(doc, [
        "单张图片打开、缩放、平移、适应窗口",
        "矩形 / 椭圆 ROI",
        "拖拽处理链：二值化、形态学、滤波等；灰度/伪彩色可作为扩展",
        "实时预览、总耗时、结果导出、原图/结果对比",
    ])
    add_para(doc, "范围外（本月明确不做）：", bold=True)
    add_bullets(doc, [
        "视频/摄像头实时流、深度学习推理、云协作",
        "工业产线检测整机、完整商业级图像工作站",
    ])

    # 2 需求
    add_heading(doc, "2 功能与非功能需求", 1)
    add_heading(doc, "2.1 功能需求总览", 2)
    add_table(
        doc,
        ["编号", "模块", "功能摘要", "优先级"],
        [
            ["F1", "图片浏览", "打开图片；滚轮缩放；拖拽平移；适应窗口", "P0"],
            ["F2", "ROI 标注", "矩形、椭圆；移动、缩放、删除", "P0"],
            ["F3", "二值化", "阈值、Otsu、使能开关", "P0"],
            ["F4", "形态学", "膨胀/腐蚀/开闭；核大小；次数", "P0"],
            ["F5", "滤波", "均值/高斯/中值等；核大小；次数", "P0"],
            ["F6", "灰度变换", "亮度对比度、反转、直方图均衡等", "P1"],
            ["F7", "拖拽工作流", "拖入、排序、删除、清空", "P0"],
            ["F8", "结果导出", "保存处理结果", "P1"],
            ["F9", "耗时统计", "处理链总耗时", "P0"],
            ["A1", "原图/结果切换", "一键对比（加分）", "P1"],
            ["A2", "图片信息条", "宽高、通道、缩放比（加分）", "P1"],
            ["A3", "常用模板", "一键「去噪→二值化」（加分）", "P1"],
            ["A4", "参数重置", "块内一键回默认（加分）", "P2"],
            ["A5", "简易直方图", "辅助理解二值化（加分）", "P2"],
            ["A6", "快捷键", "Ctrl+O、Delete 等（加分）", "P2"],
        ],
        [1.5, 2.5, 7, 1.5],
    )

    add_heading(doc, "2.2 核心功能说明（摘要）", 2)
    add_bullets(doc, [
        "图片浏览：支持常见图片格式；滚轮以鼠标位置为锚点缩放（约 10%～500%）；可平移与适应窗口。",
        "ROI：添加后可拖动与手柄缩放；Delete/按钮删除；选中显示手柄。",
        "处理链：从算法列表拖到右侧生成参数块；上下顺序即执行顺序；可单块删除或清空。",
        "二值化：SpinBox 调阈值；一键 Otsu；使能关闭则跳过该步。",
        "加分项优先做 A1～A3，演示时对比效果更明显。",
    ])

    add_heading(doc, "2.3 非功能需求", 2)
    add_table(
        doc,
        ["类别", "要求（说人话）"],
        [
            ["性能", "普通照片打开不卡；调参后尽快出结果（目标约 100ms 内有反馈）"],
            ["易用性", "默认参数合理；中文提示；手柄光标有反馈"],
            ["可靠性", "无图时操作不崩溃；ROI 拖出边界有限制"],
            ["可扩展性", "新算法按「算法文件 + Block + 列表注册」加入"],
            ["可维护性", "界面/调度/算法分层，目录清晰"],
        ],
        [2.5, 11],
    )

    # 3 用例
    add_heading(doc, "3 用例模型", 1)
    add_heading(doc, "3.1 参与者", 2)
    add_para(doc, "主要参与者：使用者（学习者 / 算法验证人员）。外部系统主要是本机文件系统（读图、写结果）。")

    add_heading(doc, "3.2 用例图", 2)
    add_picture(doc, figures["usecase"], 6.2)
    add_caption(doc, "图 3-1 用例图")

    add_heading(doc, "3.3 关键用例简述", 2)
    add_table(
        doc,
        ["用例", "前置", "主成功场景", "后置"],
        [
            ["打开/浏览图片", "程序已启动", "选文件→加载→适应窗口→可缩放平移", "场景中显示图像"],
            ["标注 ROI", "已加载图像", "选形状→添加→拖动手柄调整", "ROI 进入处理坐标"],
            ["构建处理链", "已加载图像", "从列表拖入算法→生成参数块", "右栏出现处理块"],
            ["调参预览", "已有处理块", "改参数/点 Otsu→自动重算", "预览更新，耗时刷新"],
            ["对比与导出", "已有结果", "切换原图/结果→另存为", "得到结果文件"],
        ],
        [2.5, 2.5, 5.5, 3],
    )

    # 4 架构 4+1
    add_heading(doc, "4 系统架构：4+1 视图", 1)
    add_para(
        doc,
        "按 Kruchten 4+1 视图描述架构（IPD 概念阶段的技术基线）。"
        "下面每张图都用比较直白的话解释「这一视图在看什么」。",
    )

    add_heading(doc, "4.1 逻辑视图", 2)
    add_para(doc, "看什么：系统按职责分成几层，各层干什么。")
    add_picture(doc, figures["logical"], 6.2)
    add_caption(doc, "图 4-1 逻辑视图（分层）")
    add_table(
        doc,
        ["层", "模块", "职责"],
        [
            ["表现层", "Widget / UI / ROI Item", "界面、事件、拖放、ROI 交互"],
            ["应用层", "ImageProcessor", "原图/结果管理、块调度、耗时、信号"],
            ["领域层", "BaseBlock + 各算法块", "参数 UI + process(input, roi)"],
            ["基础设施", "utils / algorithms / config", "转换、计时、OpenCV、常量"],
        ],
        [2.5, 4.5, 6.5],
    )

    add_heading(doc, "4.2 开发视图", 2)
    add_para(doc, "看什么：源码目录怎么分，依赖方向是什么。")
    add_picture(doc, figures["development"], 5.8)
    add_caption(doc, "图 4-2 开发视图")

    add_heading(doc, "4.3 进程视图", 2)
    add_para(doc, "看什么：运行时谁触发谁，处理链怎么跑。")
    add_picture(doc, figures["process"], 6.2)
    add_caption(doc, "图 4-3 进程视图")

    add_heading(doc, "4.4 物理视图", 2)
    add_para(doc, "看什么：装在哪、依赖什么库、输入输出是什么。")
    add_picture(doc, figures["physical"], 5.8)
    add_caption(doc, "图 4-4 物理视图")

    add_heading(doc, "4.5 场景视图（+1）", 2)
    add_para(doc, "看什么：一个典型故事把各视图串起来。")
    add_picture(doc, figures["scenario"], 6.2)
    add_caption(doc, "图 4-5 场景视图：二值化调参")

    # 5 UI
    add_heading(doc, "5 界面与交互设计", 1)
    add_heading(doc, "5.1 主界面信息架构", 2)
    add_para(doc, "三栏：左看图、中选算法、右配参数。符合「从左到右完成一次处理」的习惯。")
    add_picture(doc, figures["ui"], 6.4)
    add_caption(doc, "图 5-1 主界面线框")

    add_heading(doc, "5.2 视觉与交互约定", 2)
    add_table(
        doc,
        ["项目", "约定"],
        [
            ["主色", "绿色 #4CAF50（主按钮、列表高亮）"],
            ["次要操作", "蓝色 #2196F3（Otsu、对比切换）"],
            ["危险操作", "红色系（删除）"],
            ["缩放/平移", "滚轮缩放；中键或空格+拖拽平移"],
            ["ROI", "选中显示手柄；未选中隐藏"],
            ["反馈", "调参后刷新预览；底部显示耗时与提示"],
        ],
        [3, 10.5],
    )

    add_para(doc, "CDCP 建议结论：概念成立，范围可锁定；架构采用「分层 + BaseBlock 策略扩展」，进入 PDCP 计划执行。", bold=True)

    # ========== PDCP ==========
    add_heading(doc, "【IPD-PDCP】计划阶段：把事情做完、做对", 1)
    add_para(doc, "计划阶段目标：把版本、周计划、模块落地路径、验收标准和风险应对写清楚，便于跟踪。")

    add_heading(doc, "6 模块划分与扩展机制", 1)
    add_table(
        doc,
        ["目录", "放什么"],
        [
            ["core/", "主窗口 Widget、ImageProcessor 调度"],
            ["roi/", "可缩放矩形 / 椭圆图形项"],
            ["blocks/", "处理块基类与各类参数面板"],
            ["algorithms/", "OpenCV 算法实现（尽量纯逻辑）"],
            ["utils/", "图片格式转换、耗时统计"],
            ["config/", "全局常量"],
        ],
        [3, 10.5],
    )
    add_heading(doc, "6.1 扩展新算法的标准步骤（项目小框架）", 2)
    add_bullets(doc, [
        "在 algorithms/ 写纯算法函数",
        "在 blocks/ 新建 XxxBlock，继承 BaseBlock，做参数控件，重写 process()",
        "在算法列表注册，拖拽时创建对应 Block",
        "参数变化时 emit paramsChanged()，触发 ImageProcessor 重算",
    ])

    add_heading(doc, "7 数据设计与关键流程", 1)
    add_table(
        doc,
        ["数据", "说明"],
        [
            ["原始图片", "始终保留，重算都从原图开始"],
            ["当前结果", "处理链输出，用于预览与导出"],
            ["处理块列表", "有序；使能关闭的块跳过"],
            ["ROI", "场景坐标；空表示全图"],
            ["耗时", "整链计时，显示在状态区"],
        ],
        [3, 10.5],
    )
    add_para(doc, "主数据流：打开图 →（可选）画 ROI → 拖入块并调参 → 顺序 process → 刷新预览/耗时 → 可切换原图对比 → 可导出。")

    add_heading(doc, "8 项目计划与版本路线（PDCP）", 1)
    add_heading(doc, "8.1 版本路线", 2)
    add_table(
        doc,
        ["版本", "主题", "范围摘要", "建议周期"],
        [
            ["V1.0", "可用闭环", "浏览、ROI、二值化+Otsu、拖拽基础、总耗时、原图对比", "第 1～2 周"],
            ["V1.1", "算法充实", "形态学、滤波、删除/清空、模板", "第 3 周"],
            ["V1.2", "体验打磨", "信息条、导出、快捷键、自测与演示材料", "第 4 周"],
            ["V1.3+", "扩展方向", "灰度/伪彩色、直方图、单步耗时、批处理", "后续"],
        ],
        [1.8, 2.5, 6.2, 2.5],
    )

    add_heading(doc, "8.2 四周里程碑（入职第一个月）", 2)
    add_table(
        doc,
        ["周次", "目标", "主要交付"],
        [
            ["第 1 周", "能打开图、能缩放平移", "主界面骨架 + Graphics View"],
            ["第 2 周", "ROI + 二值化闭环", "矩形/椭圆 ROI；二值化块；实时预览"],
            ["第 3 周", "拖拽多算法链", "形态学/滤波；清空删除；模板"],
            ["第 4 周", "打磨 + 验收演示", "对比切换、信息条、导出、自测"],
        ],
        [2, 4.5, 7],
    )

    add_heading(doc, "8.3 技术基线", 2)
    add_bullets(doc, [
        "C++17 + Qt 6 Widgets + OpenCV 4.x + qmake",
        "架构基线：分层 + BaseBlock 策略扩展",
        "交互基线：三栏布局 + 拖拽处理链 + ROI",
    ])

    add_heading(doc, "9 质量、风险与验收", 1)
    add_heading(doc, "9.1 自测 / 验收清单", 2)
    add_table(
        doc,
        ["编号", "检查项", "通过标准"],
        [
            ["T1", "打开常见图片", "jpg/png/bmp 正常显示"],
            ["T2", "缩放平移", "不花屏、不崩溃，有缩放上下限"],
            ["T3", "ROI 增删改", "移动缩放流畅，删除后不再生效"],
            ["T4", "二值化调参", "阈值即时反馈；Otsu 可用"],
            ["T5", "多块串联", "顺序正确，效果可叠加"],
            ["T6", "使能与清空", "关使能跳过；清空恢复原图"],
            ["T7", "对比与导出", "能切换原图/结果；能保存文件"],
            ["T8", "异常操作", "无图时操作有提示或安全忽略"],
        ],
        [1.5, 4, 8],
    )

    add_heading(doc, "9.2 主要风险与应对", 2)
    add_table(
        doc,
        ["风险", "影响", "应对"],
        [
            ["功能想太多做不完", "延期", "严格 P0/P1；先保二值化闭环"],
            ["大图调参卡顿", "体验差", "先正确；有空再节流/缩小预览"],
            ["ROI 坐标对不齐", "结果错", "统一场景坐标，缩放后重点测"],
            ["参数组合过多", "演示不稳", "合理默认值 + 重置 + 模板"],
        ],
        [4, 2.5, 7],
    )

    add_heading(doc, "9.3 给领导演示的脚本（5 分钟）", 2)
    add_table(
        doc,
        ["步骤", "操作", "期望"],
        [
            ["1", "打开测试图", "适应窗口，信息条有宽高"],
            ["2", "添加 ROI 并调整", "可拖可缩放"],
            ["3", "拖入二值化并调参/Otsu", "ROI 内实时变化"],
            ["4", "拖入滤波或点模板", "处理链叠加"],
            ["5", "原图/结果切换 + 看耗时 + 保存", "对比清楚，能导出"],
        ],
        [1.5, 5.5, 6.5],
    )

    add_heading(doc, "10 加分能力与后续规划", 1)
    add_para(doc, "在 IPD 计划中，把「让 Demo 更好看」的能力单列，避免和 P0 核心闭环抢时间。")
    add_table(
        doc,
        ["编号", "能力", "价值", "建议阶段"],
        [
            ["A1", "原图/结果切换", "演示对比强", "V1.0"],
            ["A2", "图片信息条", "界面更完整", "V1.0/V1.2"],
            ["A3", "常用模板", "体现处理链价值", "V1.1"],
            ["A4", "参数重置", "演示更顺", "V1.2"],
            ["A5", "简易直方图", "和二值化搭配", "V1.3+"],
            ["A6", "快捷键", "更像正式软件", "V1.2"],
        ],
        [1.5, 3.5, 4.5, 3],
    )

    add_heading(doc, "11 总结与 IPD 决策建议", 1)
    add_bullets(doc, [
        "CDCP：产品概念清晰（学习/验证型可视化工具），范围可锁定，建议概念通过。",
        "PDCP：四周版本路线与验收清单可执行，建议按 V1.0→V1.2 推进。",
        "技术基线：分层 + BaseBlock；交互基线：三栏 + 拖拽链 + ROI。",
        "本月成功标准：现场 5 分钟演示闭环，并能讲清 IPD 两段式与 4+1 框架。",
    ])

    add_heading(doc, "附录 A 术语", 1)
    add_table(
        doc,
        ["术语", "含义"],
        [
            ["IPD", "集成产品开发：先概念决策，再计划决策，再开发"],
            ["CDCP", "概念决策检查点：确认做什么、为谁做、范围"],
            ["PDCP", "计划决策检查点：确认怎么排期、怎么验收"],
            ["4+1 视图", "逻辑/开发/进程/物理 + 场景，描述架构"],
            ["ROI", "感兴趣区域"],
            ["处理链/处理块", "按顺序串联的算法步骤及其参数面板"],
            ["Otsu", "自动计算较合适的二值化阈值"],
        ],
        [3, 10.5],
    )

    add_heading(doc, "附录 B 演示素材建议", 1)
    add_bullets(doc, [
        "准备 2～3 张图：清晰物体图、偏暗图、带噪点图",
        "二值化用对比明显的图；滤波用带噪点的图",
        "提前设好一组「看起来好看」的默认参数",
    ])

    doc.save(OUT)
    shutil.copyfile(OUT, OUT_CN)
    return OUT, OUT_CN


def main():
    figures = {
        "ipd": make_ipd_framework(),
        "usecase": make_usecase(),
        "logical": make_logical(),
        "development": make_development(),
        "process": make_process(),
        "physical": make_physical(),
        "scenario": make_scenario(),
        "ui": make_ui(),
    }
    out, out_cn = build_doc(figures)
    print("OK", out)
    print("OK", out_cn)


if __name__ == "__main__":
    main()
