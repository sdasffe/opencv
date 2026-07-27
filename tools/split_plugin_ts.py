# -*- coding: utf-8 -*-
"""从 opencv_en.ts 拆出各插件独立 .ts，并补全缺失条目。"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
ts = (ROOT / "i18n" / "opencv_en.ts").read_text(encoding="utf-8")
parts = re.split(r"(?=<context>)", ts)

HEADER = '''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US">
'''
FOOTER = "</TS>\n"

MAPPING = {
    "BinarizationBlock": ("binarization", "block_binarization_en.ts"),
    "FilterBlock": ("filter", "block_filter_en.ts"),
    "GlcmBlock": ("glcm", "block_glcm_en.ts"),
    "GrayTransformBlock": ("graytransform", "block_graytransform_en.ts"),
    "MorphologyBlock": ("morphology", "block_morphology_en.ts"),
    "PseudoColorBlock": ("pseudocolor", "block_pseudocolor_en.ts"),
}

# 源码里有、主 ts 可能缺的条目
EXTRA = {
    "BinarizationBlock": [
        ("提示", "Notice"),
        ("请先打开图片", "Please open an image first"),
    ],
}


def ensure_message(ctx: str, source: str, translation: str) -> str:
    if f"<source>{source}</source>" in ctx:
        return ctx
    msg = f'''    <message>
        <source>{source}</source>
        <translation>{translation}</translation>
    </message>
'''
    return ctx.replace("</context>", msg + "</context>")


for part in parts:
    m = re.search(r"<name>(.*?)</name>", part)
    if not m:
        continue
    name = m.group(1)
    if name not in MAPPING:
        continue
    folder, filename = MAPPING[name]
    ctx = part.strip()
    if not ctx.startswith("<context>"):
        ctx = "<context>\n" + ctx
    for src, tr in EXTRA.get(name, []):
        ctx = ensure_message(ctx, src, tr)
    out = ROOT / "plugins" / folder / "i18n" / filename
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(HEADER + ctx + "\n" + FOOTER, encoding="utf-8")
    print("wrote", out)
