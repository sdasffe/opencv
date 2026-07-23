# -*- coding: utf-8 -*-
"""生成《图像处理工具》需求与设计说明（docx，可点击目录）
对照当前完整实现：多 ROI、GLCM、撤销、会话落盘、AppLogger 等。
"""

from pathlib import Path
import math
from PIL import Image, ImageDraw, ImageFont
from docx import Document
from docx.shared import Pt, Cm, RGBColor, Inches
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_LINE_SPACING, WD_TAB_ALIGNMENT, WD_TAB_LEADER
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

ROOT = Path(__file__).resolve().parent
FIG = ROOT / "figures"
FIG.mkdir(exist_ok=True)
OUT = ROOT / (
    "".join(chr(c) for c in [
        0x56FE, 0x50CF, 0x5904, 0x7406, 0x5DE5, 0x5177, 0x2D,
        0x9700, 0x6C42, 0x4E0E, 0x8BBE, 0x8BA1, 0x8BF4, 0x660E,
    ]) + ".docx"
)


# ========================= 绘图工具 =========================

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


def make_block_diagram():
    """总体模块框图：模块 + 数据流/接口"""
    w, h = 1320, 920
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small, tiny = font(22, True), font(14, True), font(12), font(11)
    d.text((28, 14), "总体模块框图（数据流与接口）", font=title_f, fill=(33, 33, 33))

    def box(x, y, ww, hh, title, lines, fill, border):
        rounded_rect(d, (x, y, x + ww, y + hh), 10, fill, border, 2)
        d.text((x + 12, y + 10), title, font=box_f, fill=border)
        for i, line in enumerate(lines):
            d.text((x + 12, y + 36 + i * 18), line, font=small, fill=(45, 45, 45))

    box(40, 55, 540, 165, "主界面模块 Widget", [
        "菜单栏(menuBar)：文件/编辑/设置/帮助/关于",
        "打开图/文件夹 · 画布 · 多 ROI · 撤销 · 会话落盘",
        "接口：createBlockByName / getAllRoiInfo / onApplyProcessing",
        "       pushUndoSnapshot / saveSessionsToDisk",
    ], "#E3F2FD", "#1565C0")

    box(610, 55, 320, 165, "算法列表 MyListWidget", [
        "左侧工具箱（6 种算法）",
        "拖出：MIME text = 块 id",
        "稳定 id（UserRole）",
        "中英切换不丢匹配",
    ], "#E8EAF6", "#3949AB")

    box(960, 55, 320, 165, "配置 / 样式 / i18n / 日志", [
        "AppConfig 常量",
        "StyleLoader + app.qss",
        "中/英 QTranslator",
        "AppLogger → logs/",
    ], "#F3E5F5", "#7B1FA2")

    box(40, 260, 620, 175, "处理调度模块 ImageProcessor", [
        "持有：原图 / 结果 / 链 m_blocks / 多 ROI m_rois",
        "接口：setOriginalImage · add/remove/moveBlock",
        "      setRois(list) · resetResultToOriginal · reprocess",
        "信号：requestReprocess / processingFinished(ms)",
    ], "#E8F5E9", "#2E7D32")

    box(700, 260, 580, 175, "算法链模块 BaseBlock 族", [
        "抽象：process(input, rois) / blockName()",
        "参数：saveParams / loadParams（JSON）",
        "信号：paramsChanged / remove / enable / copy / paste",
        "子类：二值化/形态学/滤波/灰度/伪彩/GLCM",
    ], "#FFF3E0", "#EF6C00")

    box(40, 480, 300, 155, "ROI 模块", [
        "RoiInfo + JSON 序列化",
        "Resizable*Item（可多选）",
        "RoiProcess 并集蒙版",
    ], "#FFF8E1", "#F9A825")

    box(360, 480, 320, 155, "会话 ImageSession", [
        "按图片路径存链+ROI",
        "内存 QHash + 落盘",
        "sessions/app_sessions.json",
    ], "#E0F2F1", "#00695C")

    box(700, 480, 280, 155, "算法库 algorithms/", [
        "Binarization/Morphology/Filter",
        "GrayTransform/PseudoColor",
        "Otsu / Glcm（纯 OpenCV）",
    ], "#E0F7FA", "#00838F")

    box(1000, 480, 280, 155, "工具 utils/", [
        "ImageConverter",
        "TimeMeasurer",
        "RoiProcess / AppLogger",
    ], "#FCE4EC", "#C2185B")

    draw_arrow(d, (310, 220), (310, 260), "#1565C0", 3)
    d.text((320, 228), "setOriginalImage / setRois / reprocess", font=tiny, fill=(21, 101, 192))
    draw_arrow(d, (540, 345), (700, 345), "#EF6C00", 3)
    d.text((545, 318), "按序 process(current, rois)", font=tiny, fill=(230, 81, 0))
    draw_arrow(d, (190, 435), (190, 480), "#F9A825", 3)
    d.text((200, 445), "QList<RoiInfo>", font=tiny, fill=(249, 168, 37))
    draw_arrow(d, (500, 435), (500, 480), "#00695C", 3)
    d.text((510, 445), "换图保存/恢复", font=tiny, fill=(0, 105, 92))

    d.text((40, 670), "主数据流：原图 → [启用 Block1.process] → … → [BlockN] → 结果图 → 画布",
          font=small, fill=(40, 40, 40))
    d.text((40, 700), "控制流：参数/换序/ROI(60ms防抖) → requestReprocess → 同步 ROI → reprocess → finished",
          font=small, fill=(40, 40, 40))
    d.text((40, 730), "会话流：换图/退出 → ImageSession(chain+rois) → app_sessions.json；撤销栈另存结构快照（≤40）",
          font=small, fill=(40, 40, 40))
    d.text((40, 770), "目录：core/ · blocks/ · algorithms/ · roi/ · utils/ · config/ · styles/ · i18n/",
          font=tiny, fill=(90, 90, 90))

    path = FIG / "20_module_block.png"
    img.save(path)
    return path


def make_flowchart():
    """主处理流程"""
    w, h = 1120, 1020
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(22, True), font(13, True), font(12)
    d.text((28, 14), "主流程图：打开图像 → 构建处理链 → 重算 → 显示", font=title_f, fill=(33, 33, 33))

    def node(x, y, ww, hh, text, fill, border, diamond=False):
        if diamond:
            cx, cy = x + ww / 2, y + hh / 2
            pts = [(cx, y), (x + ww, cy), (cx, y + hh), (x, cy)]
            d.polygon(pts, fill=fill, outline=border)
        else:
            rounded_rect(d, (x, y, x + ww, y + hh), 8, fill, border, 2)
        center_text(d, (x, y, x + ww, y + hh), text, box_f)

    cx = 360
    nodes = [
        (55, "开始 / 启动（加载会话+日志）", "#E8F5E9", "#2E7D32", False),
        (125, "打开图片或文件夹", "#E3F2FD", "#1565C0", False),
        (195, "恢复该图会话？链+多ROI", "#E0F2F1", "#00695C", False),
        (265, "是否绘制/调整 ROI？", "#FFF8E1", "#F9A825", True),
        (350, "添加多 ROI（并集生效）", "#FFF8E1", "#F9A825", False),
        (420, "拖入算法块（含 GLCM）", "#FFF3E0", "#EF6C00", False),
        (490, "createBlockByName → addBlock", "#FFF3E0", "#EF6C00", False),
        (560, "调参/使能/换序/删块/撤销", "#FFF3E0", "#EF6C00", False),
        (630, "getAllRoiInfo → setRois", "#E8F5E9", "#2E7D32", False),
        (700, "reprocess：按序 process", "#E8F5E9", "#2E7D32", False),
        (770, "processingFinished → 刷新", "#E3F2FD", "#1565C0", False),
        (840, "对比/保存/导出/换图？", "#F3E5F5", "#7B1FA2", True),
        (930, "落盘会话 / 继续调参", "#E8F5E9", "#2E7D32", False),
    ]
    for y, text, fill, border, dia in nodes:
        node(cx, y, 360, 50 if not dia else 56, text, fill, border, dia)

    for y1, y2 in [(105, 125), (175, 195), (245, 265), (321, 350), (400, 420),
                   (470, 490), (540, 560), (610, 630), (680, 700), (750, 770),
                   (826, 840), (896, 930)]:
        draw_arrow(d, (cx + 180, y1), (cx + 180, y2), (80, 80, 80), 2)

    d.line([(360, 293), (180, 293), (180, 445), (360, 445)], fill=(120, 120, 120), width=2)
    draw_arrow(d, (180, 445), (360, 445), (120, 120, 120), 2)
    d.text((110, 270), "否", font=small, fill=(120, 80, 0))
    d.text((740, 280), "是", font=small, fill=(120, 80, 0))
    d.line([(720, 293), (800, 293)], fill=(120, 120, 120), width=2)
    draw_arrow(d, (800, 293), (800, 350), (120, 120, 120), 2)
    d.line([(800, 400), (720, 400)], fill=(120, 120, 120), width=2)

    d.text((780, 500), "触发重算：", font=box_f, fill=(33, 33, 33))
    for i, t in enumerate([
        "· 拖入/删除/换序块",
        "· 块参数 / 使能变化",
        "· ROI 变化（60ms 防抖）",
        "· 点击「应用」",
        "· 导入链 / 恢复会话后",
        "· 撤销恢复后",
    ]):
        d.text((780, 530 + i * 28), t, font=small, fill=(50, 50, 50))

    d.line([(720, 868), (920, 868), (920, 585), (720, 585)], fill=(123, 31, 162), width=2)
    draw_arrow(d, (920, 585), (720, 585), (123, 31, 162), 2)
    d.text((780, 720), "是→保存/导出/换图落盘", font=small, fill=(123, 31, 162))
    d.text((780, 750), "否→回到调参", font=small, fill=(123, 31, 162))

    path = FIG / "21_main_flow.png"
    img.save(path)
    return path


def make_class_diagram():
    w, h = 1460, 980
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(22, True), font(12, True), font(11)
    d.text((30, 12), "类图（核心继承与协作）", font=title_f, fill=(33, 33, 33))

    def box(x, y, ww, hh, title, lines, fill, border):
        rounded_rect(d, (x, y, x + ww, y + hh), 8, fill, border, 2)
        d.text((x + 10, y + 8), title, font=box_f, fill=border)
        d.line([(x + 8, y + 30), (x + ww - 8, y + 30)], fill=border, width=1)
        for i, line in enumerate(lines):
            d.text((x + 10, y + 38 + i * 17), line, font=small, fill=(40, 40, 40))

    box(30, 45, 310, 190, "Widget", [
        "主窗口 / 交互总控",
        "+ loadImageFromPath()",
        "+ onApplyProcessing()",
        "+ getAllRoiInfo()",
        "+ createBlockByName()",
        "+ pushUndoSnapshot / onUndo",
        "+ 会话落盘 / 对比 / 链 JSON",
    ], "#E3F2FD", "#1565C0")
    box(380, 45, 310, 190, "ImageProcessor", [
        "处理链调度引擎",
        "+ setOriginalImage / setRois",
        "+ add/remove/moveBlock",
        "+ resetResultToOriginal()",
        "+ reprocess()",
        "sig: finished / request",
    ], "#E8F5E9", "#2E7D32")
    box(730, 45, 250, 130, "ImageSession", [
        "<<struct>>",
        "chain + rois",
        "toJson / fromJson",
    ], "#E0F2F1", "#00695C")
    box(1010, 45, 220, 130, "RoiInfo", [
        "<<struct>>",
        "Rect/Ellipse/Rot",
        "toJson / fromJson",
    ], "#FFF8E1", "#F9A825")
    box(1260, 45, 170, 130, "AppLogger", [
        "<<static>>",
        "info/warn/error",
        "logs/*.log",
    ], "#F3E5F5", "#7B1FA2")

    box(30, 280, 280, 155, "BaseBlock", [
        "<<abstract>> QWidget",
        "+ process(input, rois)*",
        "+ blockName()*",
        "+ saveParams/loadParams",
        "sig: params/remove/…",
    ], "#FFF3E0", "#EF6C00")

    blocks = [
        "Binarization", "Morphology", "Filter",
        "GrayTransform", "PseudoColor", "Glcm",
    ]
    for i, name in enumerate(blocks):
        x = 350 + (i % 3) * 230
        y = 270 + (i // 3) * 105
        extra = "直通图+特征面板" if name == "Glcm" else "UI + process()"
        box(x, y, 215, 88, name + "Block", ["extends BaseBlock", extra],
            "#FFFDE7", "#F9A825")

    box(30, 530, 210, 100, "ResizableRectItem", [
        "轴对齐矩形 ROI",
    ], "#E8F5E9", "#43A047")
    box(260, 530, 230, 100, "ResizableEllipseItem", [
        "椭圆 ROI",
    ], "#E8F5E9", "#43A047")
    box(510, 530, 260, 100, "ResizableRotatedRectItem", [
        "旋转矩形 ROI",
    ], "#E8F5E9", "#43A047")
    box(800, 530, 220, 100, "RoiProcess", [
        "makeMask(list)",
        "apply 并集合成",
    ], "#F3E5F5", "#7B1FA2")
    box(1050, 530, 200, 100, "ImageConverter", [
        "QPixmap ↔ Mat",
    ], "#E0F7FA", "#00838F")
    box(1270, 530, 160, 100, "TimeMeasurer", [
        "reprocess 计时",
    ], "#E0F7FA", "#00838F")

    box(30, 680, 560, 110, "algorithms/*Algorithm", [
        "Binarization / Morphology / Filter / GrayTransform",
        "PseudoColor / Otsu / Glcm — 纯函数命名空间，由 *Block::process 调用",
    ], "#ECEFF1", "#546E7A")
    box(620, 680, 380, 110, "AppConfig / StyleLoader", [
        "块名（含 GLCM）、MIME、主题色",
        "全局 QSS 加载",
    ], "#F3E5F5", "#7B1FA2")
    box(1030, 680, 400, 110, "MyListWidget", [
        "继承 QListWidget",
        "mimeData → 块稳定 id（6 项）",
    ], "#E8EAF6", "#3949AB")

    draw_arrow(d, (340, 120), (380, 120), "#1565C0", 2)
    d.text((345, 95), "拥有", font=small, fill=(21, 101, 192))
    draw_arrow(d, (185, 235), (185, 280), "#EF6C00", 2)
    draw_arrow(d, (535, 235), (850, 175), "#00695C", 2)
    d.text((700, 200), "会话快照", font=small, fill=(0, 105, 92))
    for i in range(6):
        x = 350 + (i % 3) * 230 + 107
        y = 270 + (i // 3) * 105
        draw_arrow(d, (170, 360), (x, y), "#EF6C00", 1)

    path = FIG / "22_class.png"
    img.save(path)
    return path


def make_block_inherit_diagram():
    """算法链继承关系特写（含 GLCM）"""
    w, h = 1380, 540
    img = Image.new("RGB", (w, h), (255, 255, 255))
    d = ImageDraw.Draw(img)
    title_f, box_f, small = font(22, True), font(13, True), font(11)
    d.text((28, 14), "算法链模块：BaseBlock 继承关系（6 子类）", font=title_f, fill=(33, 33, 33))

    def box(x, y, ww, hh, title, lines, fill, border):
        rounded_rect(d, (x, y, x + ww, y + hh), 8, fill, border, 2)
        d.text((x + 12, y + 10), title, font=box_f, fill=border)
        for i, line in enumerate(lines):
            d.text((x + 12, y + 36 + i * 18), line, font=small, fill=(45, 45, 45))

    box(500, 50, 360, 100, "QWidget", ["Qt 控件基类"], "#ECEFF1", "#546E7A")
    box(440, 190, 480, 120, "BaseBlock（抽象）", [
        "process(input, rois)*  ·  blockName()*",
        "saveParams / loadParams · 使能/删除 UI",
        "paramsChanged / remove / copy / paste …",
    ], "#FFF3E0", "#EF6C00")
    draw_arrow(d, (680, 150), (680, 190), "#546E7A", 2)

    children = [
        ("BinarizationBlock", "阈值 + Otsu"),
        ("MorphologyBlock", "膨胀腐蚀开闭等"),
        ("FilterBlock", "均值/高斯/边缘"),
        ("GrayTransformBlock", "亮度对比度等"),
        ("PseudoColorBlock", "伪彩色映射"),
        ("GlcmBlock", "纹理特征直通"),
    ]
    for i, (name, desc) in enumerate(children):
        x = 20 + i * 225
        box(x, 380, 215, 95, name, [desc, "→ *Algorithm"], "#FFFDE7", "#F9A825")
        draw_arrow(d, (680, 310), (x + 107, 380), "#EF6C00", 2)

    path = FIG / "23_block_inherit.png"
    img.save(path)
    return path


# ========================= Word 工具 =========================

def set_run_font(run, size=11, bold=False, name="微软雅黑", color=None):
    run.font.size = Pt(size)
    run.font.bold = bold
    run.font.name = name
    run._element.rPr.rFonts.set(qn("w:eastAsia"), name)
    if color:
        run.font.color.rgb = RGBColor(*color)


def set_cell_shading(cell, hex_color):
    tc = cell._tc
    tcPr = tc.get_or_add_tcPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:fill"), hex_color)
    shd.set(qn("w:val"), "clear")
    tcPr.append(shd)


def add_bookmark(paragraph, bookmark_name, bookmark_id):
    tag = paragraph._p
    start = OxmlElement("w:bookmarkStart")
    start.set(qn("w:id"), str(bookmark_id))
    start.set(qn("w:name"), bookmark_name)
    end = OxmlElement("w:bookmarkEnd")
    end.set(qn("w:id"), str(bookmark_id))
    tag.insert(0, start)
    tag.append(end)


def add_heading(doc, text, level=1, bookmark_name=None, bookmark_id=None):
    p = doc.add_heading(text, level=level)
    for run in p.runs:
        set_run_font(run, size=16 if level == 1 else (13 if level == 2 else 12),
                     bold=True, name="微软雅黑")
    if bookmark_name is not None and bookmark_id is not None:
        add_bookmark(p, bookmark_name, bookmark_id)
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


def add_table(doc, headers, rows, col_widths=None):
    table = doc.add_table(rows=1 + len(rows), cols=len(headers))
    table.style = "Table Grid"
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    for i, h in enumerate(headers):
        cell = table.rows[0].cells[i]
        cell.text = ""
        run = cell.paragraphs[0].add_run(h)
        set_run_font(run, size=10, bold=True, color=(255, 255, 255))
        set_cell_shading(cell, "0F766E")
    for r_i, row in enumerate(rows):
        for c_i, val in enumerate(row):
            cell = table.rows[r_i + 1].cells[c_i]
            cell.text = ""
            run = cell.paragraphs[0].add_run(str(val))
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


def add_hyperlink_to_bookmark(paragraph, text, bookmark_name):
    hyperlink = OxmlElement("w:hyperlink")
    hyperlink.set(qn("w:anchor"), bookmark_name)
    new_run = OxmlElement("w:r")
    rPr = OxmlElement("w:rPr")
    color = OxmlElement("w:color")
    color.set(qn("w:val"), "0563C1")
    rPr.append(color)
    u = OxmlElement("w:u")
    u.set(qn("w:val"), "single")
    rPr.append(u)
    sz = OxmlElement("w:sz")
    sz.set(qn("w:val"), "24")
    rPr.append(sz)
    rFonts = OxmlElement("w:rFonts")
    rFonts.set(qn("w:ascii"), "微软雅黑")
    rFonts.set(qn("w:hAnsi"), "微软雅黑")
    rFonts.set(qn("w:eastAsia"), "微软雅黑")
    rPr.append(rFonts)
    new_run.append(rPr)
    text_elem = OxmlElement("w:t")
    text_elem.text = text
    new_run.append(text_elem)
    hyperlink.append(new_run)
    paragraph._p.append(hyperlink)


def add_toc_entry(doc, title, bookmark, level=1):
    p = doc.add_paragraph()
    p.paragraph_format.space_after = Pt(4)
    p.paragraph_format.space_before = Pt(2)
    p.paragraph_format.left_indent = Cm((level - 1) * 0.75)
    tab_stops = p.paragraph_format.tab_stops
    tab_stops.add_tab_stop(Cm(14.5), WD_TAB_ALIGNMENT.RIGHT, WD_TAB_LEADER.DOTS)
    add_hyperlink_to_bookmark(p, title, bookmark)
    return p


def enable_update_fields_on_open(doc):
    settings = doc.settings.element
    update = OxmlElement("w:updateFields")
    update.set(qn("w:val"), "true")
    settings.append(update)


# ========================= 正文 =========================

def build_doc(figures):
    doc = Document()
    for section in doc.sections:
        section.top_margin = Cm(2.2)
        section.bottom_margin = Cm(2.2)
        section.left_margin = Cm(2.4)
        section.right_margin = Cm(2.4)
    enable_update_fields_on_open(doc)

    bm = {
        "ch1": ("bm_ch1", 1),
        "s11": ("bm_s11", 2),
        "s12": ("bm_s12", 3),
        "s13": ("bm_s13", 4),
        "s14": ("bm_s14", 5),
        "ch2": ("bm_ch2", 6),
        "s21": ("bm_s21", 7),
        "s22": ("bm_s22", 8),
        "s221": ("bm_s221", 9),
        "s222": ("bm_s222", 10),
        "s223": ("bm_s223", 11),
        "s224": ("bm_s224", 12),
        "s225": ("bm_s225", 13),
        "s226": ("bm_s226", 14),
        "s227": ("bm_s227", 15),
        "s23": ("bm_s23", 16),
        "s24": ("bm_s24", 17),
        "app": ("bm_app", 18),
    }

    for _ in range(2):
        doc.add_paragraph()
    add_para(doc, "图像处理工具", size=28, bold=True, center=True, space_after=12)
    add_para(doc, "需求与设计说明", size=20, bold=True, center=True, space_after=10)
    add_para(doc, "技术栈：C++17 / Qt 6 Widgets / OpenCV 4.x / qmake", size=11, center=True)
    add_para(doc, "工程名：opencv　　版本：1.0.0　　文档日期：2026-07-22", size=11, center=True, space_after=16)
    add_para(doc,
             "本文结构：第一章给出功能/性能/DFX 需求并转化为规格；"
             "第二章给出概要与详细设计（模块框图、模块说明、流程图、类图）。"
             "内容与当前完整实现同步（含多 ROI、GLCM、撤销、会话落盘、运行日志）。",
             size=10, center=True)
    doc.add_page_break()

    add_para(doc, "目录", size=18, bold=True, center=True, space_after=12)
    add_para(doc, "（点击条目可跳转到对应章节）", size=10, center=True, space_after=10)

    toc_items = [
        ("第一章 需求", "bm_ch1", 1),
        ("1.1 功能需求", "bm_s11", 2),
        ("1.2 性能需求", "bm_s12", 2),
        ("1.3 DFX 需求", "bm_s13", 2),
        ("1.4 需求转化为规格", "bm_s14", 2),
        ("第二章 概要设计与详细设计", "bm_ch2", 1),
        ("2.1 总体结构与模块框图", "bm_s21", 2),
        ("2.2 模块说明", "bm_s22", 2),
        ("2.2.1 主界面模块 Widget", "bm_s221", 3),
        ("2.2.2 处理调度模块 ImageProcessor", "bm_s222", 3),
        ("2.2.3 算法链模块 BaseBlock 族", "bm_s223", 3),
        ("2.2.4 ROI 模块", "bm_s224", 3),
        ("2.2.5 会话与撤销", "bm_s225", 3),
        ("2.2.6 算法库与工具", "bm_s226", 3),
        ("2.2.7 配置、样式、国际化与日志", "bm_s227", 3),
        ("2.3 流程图", "bm_s23", 2),
        ("2.4 类图", "bm_s24", 2),
        ("附录 术语", "bm_app", 1),
    ]
    for title, bookmark, level in toc_items:
        add_toc_entry(doc, title, bookmark, level)

    doc.add_page_break()

    # =====================================================================
    # 第一章
    # =====================================================================
    add_heading(doc, "第一章 需求", 1, *bm["ch1"])
    add_para(doc,
             "本章描述本工具应满足的功能需求、性能需求与常规 DFX 需求。"
             "需求表统一三列：编号、需求描述、说明。最后将需求转化为可对照实现的规格条目。")

    add_heading(doc, "1.1 功能需求", 2, *bm["s11"])
    add_table(doc, ["编号", "需求描述", "说明"], [
        ["FR-01", "打开单张图片并在画布显示", "支持 png/jpg/jpeg/bmp/gif/tiff"],
        ["FR-02", "打开文件夹并以缩略图浏览切换", "点击缩略图切换当前图；每张图独立会话"],
        ["FR-03", "画布缩放与平移", "滚轮缩放（0.1～5.0）；中键/空白拖拽平移；Ctrl+0 适应窗口"],
        ["FR-04", "显示图像信息与处理耗时", "宽高、缩放比例；重算后显示毫秒耗时"],
        ["FR-05", "从算法列表拖拽创建处理块", "左侧工具箱拖到右侧处理链面板"],
        ["FR-06", "处理链按顺序流水线执行", "仅执行「启用」块；顺序即执行顺序"],
        ["FR-07", "二值化处理（含 Otsu）", "灰度∈[下限,上限]为白；一键 Otsu 写回阈值"],
        ["FR-08", "形态学处理", "膨胀/腐蚀/开/闭/顶帽/底帽/梯度；核与迭代可调"],
        ["FR-09", "滤波处理", "均值/高斯/中值及 Sobel/Laplacian/Prewitt/Roberts"],
        ["FR-10", "灰度变换", "转灰、亮度对比度、反色、对数、伽马、均衡化、归一化"],
        ["FR-11", "伪彩色映射", "Jet/Hot/Cool/Rainbow 等 10 种色带"],
        ["FR-12", "灰度共生矩阵（GLCM）分析", "图像直通；面板显示对比度/相关/能量/均匀性/熵/相异性"],
        ["FR-13", "处理块可调参、使能、删除", "参数变化触发重算；可临时关闭某块"],
        ["FR-14", "处理块可拖拽换序", "右侧面板内拖动改变执行顺序"],
        ["FR-15", "处理块可复制/粘贴参数", "右键菜单；剪贴板存单块 JSON"],
        ["FR-16", "清空整条处理链", "确认后移除所有块，结果恢复为原图"],
        ["FR-17", "多 ROI：矩形/椭圆/旋转矩形", "可同时存在多个；Delete 删选中或全部"],
        ["FR-18", "多 ROI 并集局部处理", "区域外保持原像素；空列表=全图"],
        ["FR-19", "按住对比原图与结果图", "按下看原图，松开恢复结果"],
        ["FR-20", "保存当前结果图", "用户选择路径与格式（png/jpeg/bmp 等）"],
        ["FR-21", "处理链导入/导出为 JSON", "导出含块类型与参数；导入替换现有链"],
        ["FR-22", "按图片路径记忆处理链与 ROI", "换图自动保存/恢复；退出写入 sessions/app_sessions.json"],
        ["FR-23", "撤销结构变更（Ctrl+Z）", "撤销加删块、换序、ROI 增减、清空/导入等；深度≤40"],
        ["FR-24", "界面中/英切换", "设置菜单切换；语言偏好可持久化"],
        ["FR-25", "运行日志与关于/帮助", "日志写入 exe/logs/；关于可查看日志与会话路径；快捷键帮助"],
    ], [2.2, 6.5, 6.5])

    add_heading(doc, "1.2 性能需求", 2, *bm["s12"])
    add_table(doc, ["编号", "需求描述", "说明"], [
        ["PR-01", "调参后自动重算并尽快反馈", "常规分辨率（约 1920×1080）交互可接受"],
        ["PR-02", "ROI 拖动时防抖重算", "几何变化经约 60ms 防抖后再重算，避免拖动卡顿"],
        ["PR-03", "重算耗时可观测", "界面显示最近一次 reprocess 墙钟毫秒"],
        ["PR-04", "缩放浏览不触发算法链", "仅改变视图变换矩阵"],
        ["PR-05", "撤销栈有上限", "单图最多保留 40 层结构快照，避免内存无限增长"],
        ["PR-06", "重算在调用线程完成并可感知", "完成后发信号刷新 UI（本版不强制后台线程池）"],
    ], [2.2, 6.5, 6.5])

    add_heading(doc, "1.3 DFX 需求", 2, *bm["s13"])
    add_para(doc, "本节采用常规、简化的 DFX（可靠性、可维护性、易用性、可扩展性、兼容性）。")
    add_table(doc, ["编号", "需求描述", "说明"], [
        ["DX-01", "可靠性：空图/非法文件不崩溃", "打开失败提示；无图时重算安全返回"],
        ["DX-02", "可靠性：参数越界由控件约束", "SpinBox/组合框限制合法范围"],
        ["DX-03", "可靠性：关键操作可追踪", "AppLogger 记录打开、保存、导入导出、异常等"],
        ["DX-04", "可维护性：算法与 UI 分层", "algorithms 无 Qt UI；Block 负责控件与调用"],
        ["DX-05", "可维护性：配置与会话集中", "AppConfig；ImageSession 统一链+ROI 快照格式"],
        ["DX-06", "易用性：拖拽建链、按住对比、撤销", "降低调参与试错成本"],
        ["DX-07", "易用性：中英界面与快捷键说明", "降低上手成本"],
        ["DX-08", "可扩展性：新增算法块有固定步骤", "继承 BaseBlock + 登记块名即可扩展"],
        ["DX-09", "兼容性：Windows + Qt6 + OpenCV4", "以当前 qmake 工程为交付基线"],
    ], [2.2, 6.5, 6.5])

    add_heading(doc, "1.4 需求转化为规格", 2, *bm["s14"])
    add_para(doc, "下表将需求转化为可实现、可验收的规格，便于追溯到第二章与源码。")
    add_table(doc, ["需求编号", "规格编号", "规格描述"], [
        ["FR-01", "SP-01", "「打开图片」→ loadImageFromPath 成功则 setOriginalImage 并刷新画布"],
        ["FR-02", "SP-02", "「打开文件夹」填充缩略图；点击切换前 saveCurrentSession，切换后按路径 restoreSession"],
        ["FR-03", "SP-03", "滚轮缩放限制 MIN/MAX_SCALE；支持平移与 fitViewToImage（Ctrl+0）"],
        ["FR-04", "SP-04", "信息标签显示宽高/缩放；processingFinished(ms) 更新耗时标签"],
        ["FR-05", "SP-05", "MyListWidget 拖出稳定块 id；Drop → createBlockByName → addBlockToPanel"],
        ["FR-06", "SP-06", "ImageProcessor::reprocess 按序调用启用块 process(current, m_rois)"],
        ["FR-07", "SP-07", "BinarizationBlock 上下限；Otsu 用原图/ROI 并集外接框计算并写回"],
        ["FR-08", "SP-08", "MorphologyBlock → MorphologyAlgorithm::apply（Op/核/迭代）"],
        ["FR-09", "SP-09", "FilterBlock → FilterAlgorithm::apply"],
        ["FR-10", "SP-10", "GrayTransformBlock → GrayTransformAlgorithm::apply"],
        ["FR-11", "SP-11", "PseudoColorBlock → PseudoColorAlgorithm::apply"],
        ["FR-12", "SP-12", "GlcmBlock：process 返回原图；GlcmAlgorithm::compute 刷新特征标签"],
        ["FR-13", "SP-13", "BaseBlock 使能/删除；paramsChanged → requestReprocess"],
        ["FR-14", "SP-14", "MIME_BLOCK_REORDER + moveBlock / reorderBlock 同步布局与链序"],
        ["FR-15", "SP-15", "MIME_BLOCK_CLIPBOARD 存单块 JSON；面板空白处可粘贴追加"],
        ["FR-16", "SP-16", "clearAllBlocks + resetResultToOriginal，画布恢复原图"],
        ["FR-17", "SP-17", "三类 Resizable*Item 可并存多份；getAllRoiInfo 收集列表"],
        ["FR-18", "SP-18", "RoiProcess::apply(list) 蒙版并集；空列表=全图"],
        ["FR-19", "SP-19", "对比按钮 pressed=originalImage，released=resultImage"],
        ["FR-20", "SP-20", "保存对话框写出 resultImage"],
        ["FR-21", "SP-21", "导出 {version,blocks[]}；导入确认后清链重建并 loadParams"],
        ["FR-22", "SP-22", "m_sessions[路径]=ImageSession；退出 saveSessionsToDisk→sessions/app_sessions.json"],
        ["FR-23", "SP-23", "结构变更前 pushUndoSnapshot；Ctrl+Z 弹出恢复链+ROI；MAX_UNDO=40"],
        ["FR-24", "SP-24", "QTranslator 装卸载；LanguageChange→retranslateDynamicUi/各块 retranslateUi；QSettings 记语言"],
        ["FR-25", "SP-25", "AppLogger::init；关于对话框展示日志与会话路径；帮助列出快捷键"],
        ["PR-01", "SP-26", "参数变更直接进入 onApplyProcessing，无额外确认"],
        ["PR-02", "SP-27", "scene.changed → QTimer(约60ms) → onApplyProcessing"],
        ["PR-03", "SP-28", "reprocess 用 TimeMeasurer，经 processingFinished 上报"],
        ["PR-04", "SP-29", "缩放只改视图变换，不调用 reprocess"],
        ["PR-05", "SP-30", "undo 栈超出 MAX_UNDO 时丢弃最旧快照"],
        ["PR-06", "SP-31", "重算在调用线程同步完成，结束后发信号刷新 UI"],
        ["DX-01", "SP-32", "加载失败提示；hasImage 为假时 reprocess 返回 0"],
        ["DX-02", "SP-33", "控件 range 约束；核偶数在算法侧修正为奇数"],
        ["DX-03", "SP-34", "关键路径调用 AppLogger::info/warn/error"],
        ["DX-04", "SP-35", "目录分层 core/blocks/algorithms/roi/utils/config"],
        ["DX-05", "SP-36", "AppConfig 集中常量；ImageSession 统一会话 JSON schema"],
        ["DX-06", "SP-37", "三栏布局；对比为按住查看；文件菜单提供撤销"],
        ["DX-07", "SP-38", "设置可切换中/英；帮助列出 Delete/Ctrl+Z/Ctrl+0 等"],
        ["DX-08", "SP-39", "新块：继承 BaseBlock → 实现 process/blockName → AppConfig+createBlockByName 登记"],
        ["DX-09", "SP-40", "交付基线：Windows 桌面、Qt 6 Widgets、OpenCV 4、qmake"],
    ], [2.4, 2.4, 10.4])

    doc.add_page_break()

    # =====================================================================
    # 第二章
    # =====================================================================
    add_heading(doc, "第二章 概要设计与详细设计", 1, *bm["ch2"])
    add_para(doc,
             "本章在规格基础上给出模块划分、数据流与接口，说明各模块职责与实现要点，"
             "并给出主流程图与类图。")

    add_heading(doc, "2.1 总体结构与模块框图", 2, *bm["s21"])
    add_para(doc,
             "系统按「表现层 → 调度层 → 算法块层 → 算法/工具/会话层」组织。"
             "Widget 负责交互、多 ROI、撤销与会话；ImageProcessor 按链调度；"
             "BaseBlock 子类（含 GLCM）负责参数 UI 与单步处理；"
             "algorithms / RoiProcess / AppLogger 提供计算与基础设施。")
    add_picture(doc, figures["block"], 6.4)
    add_caption(doc, "图 2-1 总体模块框图（含主要数据流与接口）")

    add_para(doc, "主要接口约定：", bold=True)
    add_table(doc, ["接口/数据", "提供方", "消费方 / 说明"], [
        ["QPixmap 原图/结果图", "ImageProcessor", "Widget 显示；链输入输出"],
        ["QList<RoiInfo>", "Widget::getAllRoiInfo", "setRois → Block::process；空=全图"],
        ["BaseBlock*", "createBlockByName", "面板布局 + ImageProcessor::addBlock"],
        ["process(input, rois)", "各 *Block", "reprocess 顺序调用"],
        ["paramsChanged", "BaseBlock", "→ requestReprocess"],
        ["requestReprocess", "ImageProcessor", "Widget 先同步 ROI 再 reprocess"],
        ["processingFinished(ms)", "ImageProcessor", "刷新画布与耗时"],
        ["ImageSession", "Widget", "换图记忆；撤销快照；落盘 JSON"],
        ["QJsonObject 块参数", "saveParams/loadParams", "链导入导出、复制粘贴、会话"],
        ["cv::Mat", "ImageConverter", "algorithms / RoiProcess"],
        ["日志事件", "各模块", "AppLogger → exe/logs/"],
    ], [4.5, 4.5, 6.5])

    add_heading(doc, "2.2 模块说明", 2, *bm["s22"])

    add_heading(doc, "2.2.1 主界面模块 Widget", 3, *bm["s221"])
    add_para(doc,
             "位置：core/widget.h/.cpp + widget.ui。"
             "职责：文件打开与文件夹缩略图、画布、多 ROI 图元、处理链面板、"
             "菜单（文件含撤销、设置含语言）、对比/保存/链 JSON、会话与撤销、中英切换。"
             "统一重算入口 onApplyProcessing：getAllRoiInfo → setRois → reprocess。"
             "相关类：MyListWidget、三种 Resizable*Item、各 BaseBlock 子类、ImageSession。")

    add_heading(doc, "2.2.2 处理调度模块 ImageProcessor", 3, *bm["s222"])
    add_para(doc,
             "位置：core/imageprocessor.h/.cpp，无界面。"
             "持有 m_original、m_result、m_blocks、m_rois（列表）。"
             "结构变化只发 requestReprocess；reprocess 内 TimeMeasurer 计时。"
             "resetResultToOriginal 用于清空链后把结果复位为原图。")

    add_heading(doc, "2.2.3 算法链模块 BaseBlock 族", 3, *bm["s223"])
    add_para(doc,
             "位置：blocks/。"
             "BaseBlock 提供标题栏、使能/删除、拖拽换序、右键复制粘贴，"
             "纯虚接口 process(input, rois) / blockName。"
             "六子类：Binarization / Morphology / Filter / GrayTransform / PseudoColor / Glcm。"
             "GLCM 为分析块：图像原样通过，面板展示纹理统计（量化级、距离可调）。")
    add_picture(doc, figures["inherit"], 6.3)
    add_caption(doc, "图 2-2 算法链 BaseBlock 继承关系")
    add_table(doc, ["类", "父类/关系", "要点"], [
        ["BaseBlock", "QWidget", "抽象块；UI 骨架与信号"],
        ["BinarizationBlock", "BaseBlock", "上下限 + Otsu"],
        ["MorphologyBlock", "BaseBlock", "Op / 核 / 迭代"],
        ["FilterBlock", "BaseBlock", "滤波类型与核"],
        ["GrayTransformBlock", "BaseBlock", "变换类型与亮度等"],
        ["PseudoColorBlock", "BaseBlock", "色带 Map"],
        ["GlcmBlock", "BaseBlock", "直通图 + 特征显示"],
        ["ImageProcessor", "聚合块指针", "按序 process"],
        ["Widget", "创建并拥有块", "createBlockByName"],
    ], [4, 4, 7.5])

    add_heading(doc, "2.2.4 ROI 模块", 3, *bm["s224"])
    add_para(doc,
             "RoiInfo 描述 Rect / Ellipse / RotatedRect，支持 toJson/fromJson。"
             "场景中可同时存在多个 Resizable*Item；getAllRoiInfo 收集为列表。"
             "RoiProcess::makeMask/apply 对列表做蒙版并集（OR）；空列表全图处理。"
             "对应规格 SP-17、SP-18、SP-27。")

    add_heading(doc, "2.2.5 会话与撤销", 3, *bm["s225"])
    add_para(doc,
             "ImageSession（core/imagesession.h）= 处理链 JSON 数组 + ROI 列表。"
             "Widget::m_sessions 以图片绝对路径为键；换图时保存当前、恢复目标；"
             "退出写入 {exe}/sessions/app_sessions.json（version=1）。"
             "撤销：结构变更前 pushUndoSnapshot，Ctrl+Z 弹出恢复；"
             "栈深度 MAX_UNDO=40；换图清空该图撤销栈。ROI 几何微调走防抖重算，"
             "不把每一帧拖动都压入撤销栈（压栈点为添加/删除 ROI 与链结构变更）。")

    add_heading(doc, "2.2.6 算法库与工具", 3, *bm["s226"])
    add_para(doc,
             "algorithms/：Binarization、Morphology、Filter、GrayTransform、"
             "PseudoColor、Otsu、Glcm —— 均无 Qt UI。"
             "utils/：ImageConverter、TimeMeasurer、RoiProcess、AppLogger。"
             "Block::process 典型骨架：Pixmap→Mat → RoiProcess::apply(算法) → Mat→Pixmap；"
             "GlcmBlock 额外 compute 特征后仍返回原图。")

    add_heading(doc, "2.2.7 配置、样式、国际化与日志", 3, *bm["s227"])
    add_para(doc,
             "AppConfig：块名（含 BLOCK_NAME_GLCM）、默认阈值、缩放极限、MIME、主题色。"
             "StyleLoader + styles/app.qss。"
             "i18n：opencv_en.qm + QTranslator；块列表用稳定 id 抗语言切换。"
             "AppLogger：exe/logs/app_yyyyMMdd.log；关于对话框可提示路径。"
             "扩展新块：子类 + AppConfig 登记 + createBlockByName（SP-39）。")

    add_heading(doc, "2.3 流程图", 2, *bm["s23"])
    add_para(doc, "下图描述启动、打开图像、可选多 ROI、建链、统一重算到显示/落盘的主路径。")
    add_picture(doc, figures["flow"], 5.8)
    add_caption(doc, "图 2-3 主流程图")

    add_para(doc, "重算核心步骤：", bold=True)
    add_table(doc, ["步骤", "动作", "说明"], [
        ["1", "getAllRoiInfo", "空列表 = 全图"],
        ["2", "setRois(list)", "写入本次重算 ROI"],
        ["3", "current = original", "不破坏 m_original"],
        ["4", "启用块依次 process", "current = block->process(current, rois)"],
        ["5", "m_result = current", "emit processingFinished"],
        ["6", "Widget 刷新画布", "更新耗时与信息标签"],
    ], [2, 5, 8.5])

    add_heading(doc, "2.4 类图", 2, *bm["s24"])
    add_para(doc,
             "下图汇总核心类协作：Widget 拥有 ImageProcessor、会话与撤销栈；"
             "六种 Block 继承 BaseBlock；ROI/会话/日志为配套结构。")
    add_picture(doc, figures["class"], 6.4)
    add_caption(doc, "图 2-4 核心类图")

    add_table(doc, ["类/单元", "层", "职责摘要"], [
        ["Widget", "表现", "交互总控、多 ROI、链 UI、会话、撤销、语言"],
        ["MyListWidget", "表现", "算法列表拖出稳定 id"],
        ["ImageProcessor", "应用", "处理链调度与结果缓存"],
        ["BaseBlock 及 6 子类", "应用+表现", "参数 UI + process"],
        ["ImageSession", "应用", "单图链+ROI 快照与落盘格式"],
        ["RoiInfo / Resizable*Item", "领域/表现", "ROI 数据与可视化编辑"],
        ["RoiProcess", "领域", "多 ROI 并集蒙版与局部合成"],
        ["*Algorithm / Glcm", "领域", "OpenCV 算法与纹理特征"],
        ["ImageConverter / TimeMeasurer", "基础", "格式转换与计时"],
        ["AppConfig / StyleLoader / AppLogger", "基础", "配置、皮肤、文件日志"],
    ], [4.8, 2.5, 8.2])

    doc.add_page_break()
    add_heading(doc, "附录 术语", 1, *bm["app"])
    add_table(doc, ["术语", "含义", "说明"], [
        ["处理链", "按序排列的处理块", "顺序即流水线执行顺序"],
        ["处理块", "BaseBlock 子类实例", "含参数 UI 与 process"],
        ["多 ROI 并集", "多个感兴趣区域", "蒙版 OR；空=全图"],
        ["会话 ImageSession", "单图的链+ROI 快照", "内存哈希 + app_sessions.json"],
        ["撤销", "结构快照回退", "Ctrl+Z；深度≤40；不含 Redo"],
        ["GLCM", "灰度共生矩阵", "纹理特征分析块，图像直通"],
        ["重算 reprocess", "按当前链与 ROI 生成结果", "入口 onApplyProcessing"],
        ["规格 SP-xx", "由需求转化的实现约定", "见 1.4 节"],
    ], [3.2, 4.8, 7.5])

    add_para(doc,
             "（完）文档与当前源码目录 core、blocks、algorithms、roi、utils、config 对应；"
             "若实现变更，请同步更新 1.4 规格表与第二章接口说明。",
             size=10)

    doc.save(str(OUT))
    return OUT


def main():
    figures = {
        "block": make_block_diagram(),
        "flow": make_flowchart(),
        "class": make_class_diagram(),
        "inherit": make_block_inherit_diagram(),
    }
    out = build_doc(figures)
    print("OK:", out)


if __name__ == "__main__":
    main()
