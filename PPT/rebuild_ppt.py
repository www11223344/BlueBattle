#!/usr/bin/env python3
"""Rebuild BlueBattle project PPT around the current technical highlights."""

from pathlib import Path

from pptx import Presentation
from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_SHAPE
from pptx.enum.text import MSO_ANCHOR, PP_ALIGN
from pptx.util import Emu, Pt

# Match existing PPT pixel-as-point canvas (1280x720).
SLIDE_W = Emu(16256000)  # 1280pt
SLIDE_H = Emu(9144000)   # 720pt

PRIMARY = RGBColor(0x0E, 0xA5, 0xE9)
SECONDARY = RGBColor(0x64, 0x74, 0x8B)
ACCENT = RGBColor(0xF5, 0x9E, 0x0B)
BG = RGBColor(0x0F, 0x17, 0x2A)
TEXT = RGBColor(0xF8, 0xFA, 0xFC)
CARD = RGBColor(0x1E, 0x29, 0x3B)
BORDER = RGBColor(0x33, 0x41, 0x55)

FONT = "Microsoft YaHei"
OUT = Path(__file__).with_name("虚幻引擎双人对打游戏开发_修改版.pptx")


def px(v: float) -> Emu:
    return Emu(int(v * 12700))


def set_run(run, text, size, color, bold=False):
    run.text = text
    run.font.size = Pt(size)
    run.font.color.rgb = color
    run.font.bold = bold
    run.font.name = FONT


def add_bg(slide):
    shape = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, SLIDE_W, SLIDE_H)
    shape.fill.solid()
    shape.fill.fore_color.rgb = BG
    shape.line.fill.background()
    # send to back
    spTree = slide.shapes._spTree
    sp = shape._element
    spTree.remove(sp)
    spTree.insert(2, sp)
    return shape


def add_rect(slide, x, y, w, h, fill, line=None):
    shape = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, px(x), px(y), px(w), px(h))
    shape.fill.solid()
    shape.fill.fore_color.rgb = fill
    if line is None:
        shape.line.fill.background()
    else:
        shape.line.color.rgb = line
        shape.line.width = Pt(1)
    return shape


def add_text(slide, x, y, w, h, text, size=18, color=TEXT, bold=False, align=PP_ALIGN.LEFT, anchor=MSO_ANCHOR.TOP):
    box = slide.shapes.add_textbox(px(x), px(y), px(w), px(h))
    tf = box.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.alignment = align
    p.clear()
    set_run(p.add_run(), text, size, color, bold)
    return box


def add_bullets(slide, x, y, w, h, items, size=18, color=TEXT, bullet="•", spacing=8):
    box = slide.shapes.add_textbox(px(x), px(y), px(w), px(h))
    tf = box.text_frame
    tf.word_wrap = True
    for i, item in enumerate(items):
        p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
        if i == 0:
            p.clear()
        p.alignment = PP_ALIGN.LEFT
        p.space_after = Pt(spacing)
        set_run(p.add_run(), f"{bullet}  {item}", size, color, False)
    return box


def page_chrome(slide, chapter, page_no, total=15):
    add_text(slide, 60, 18, 700, 24, chapter, size=12, color=SECONDARY)
    add_text(slide, 1100, 18, 120, 24, f"{page_no:02d} / {total:02d}", size=12, color=SECONDARY, align=PP_ALIGN.RIGHT)
    add_rect(slide, 60, 44, 1160, 1, BORDER)
    add_text(slide, 60, 688, 400, 20, "BlueBattle | UE5 双人格斗", size=11, color=SECONDARY)


def title_with_bar(slide, title, y=60):
    add_rect(slide, 60, y + 6, 4, 28, PRIMARY)
    add_text(slide, 78, y, 1100, 40, title, size=28, color=TEXT, bold=True)
    add_rect(slide, 78, y + 42, 40, 2, PRIMARY)


def new_slide(prs):
    slide = prs.slides.add_slide(prs.slide_layouts[0])
    # clear any default placeholders
    for shape in list(slide.shapes):
        sp = shape._element
        sp.getparent().remove(sp)
    add_bg(slide)
    return slide


def build():
    prs = Presentation()
    prs.slide_width = SLIDE_W
    prs.slide_height = SLIDE_H

    # ---- 1 Cover ----
    s = new_slide(prs)
    add_rect(s, 0, 0, 1280, 720, BG)
    add_rect(s, 0, 310, 1280, 2, PRIMARY)
    add_text(s, 60, 220, 1160, 60, "BlueBattle", size=48, color=PRIMARY, bold=True, align=PP_ALIGN.CENTER)
    add_text(s, 60, 290, 1160, 40, "3D 双人格斗对战游戏", size=32, color=TEXT, bold=True, align=PP_ALIGN.CENTER)
    add_text(s, 60, 360, 1160, 30, "基于虚幻引擎5的本地双人对战格斗项目 | 技术架构与核心实现", size=18, color=SECONDARY, align=PP_ALIGN.CENTER)
    add_text(s, 60, 520, 1160, 24, "独立开发 · 2026.05 - 2026.06", size=16, color=SECONDARY, align=PP_ALIGN.CENTER)
    add_text(s, 60, 560, 1160, 24, "https://github.com/www11223344/BlueBattle", size=14, color=PRIMARY, align=PP_ALIGN.CENTER)

    # ---- 2 TOC ----
    s = new_slide(prs)
    page_chrome(s, "目录", 2)
    title_with_bar(s, "目录")
    toc = [
        ("01", "项目概述", "独立完成 UE5 客户端全流程，支持本地双人对战"),
        ("02", "技术亮点", "Mass / Lyra Experience / CommonUI+MVVM / 近战 Sweep 判定"),
        ("03", "架构细节", "战斗组件、Experience 加载与 UI 数据流"),
        ("04", "总结与展望", "技术收获与后续方向"),
    ]
    for i, (num, title, desc) in enumerate(toc):
        y = 140 + i * 110
        add_rect(s, 60, y, 1160, 90, CARD, BORDER)
        add_text(s, 90, y + 18, 100, 50, num, size=36, color=PRIMARY, bold=True)
        add_text(s, 220, y + 18, 900, 36, title, size=24, color=TEXT, bold=True)
        add_text(s, 220, y + 52, 900, 28, desc, size=16, color=SECONDARY)

    # ---- 3 Chapter 01 ----
    s = new_slide(prs)
    page_chrome(s, "01 项目概述", 3)
    add_text(s, 80, 240, 200, 120, "01", size=96, color=RGBColor(0x1E, 0x29, 0x3B), bold=True)
    add_text(s, 280, 280, 900, 50, "项目概述", size=40, color=TEXT, bold=True)
    add_text(s, 280, 350, 900, 40, "独立完成 UE5 客户端全流程开发，支持本地双人对战", size=20, color=SECONDARY)

    # ---- 4 Project intro ----
    s = new_slide(prs)
    page_chrome(s, "01 项目概述", 4)
    title_with_bar(s, "项目简介")
    cards = [
        ("项目", "BlueBattle — 3D 双人格斗对战游戏"),
        ("角色 / 周期", "独立开发  |  2026.05 - 2026.06"),
        ("链接", "https://github.com/www11223344/BlueBattle"),
        ("概述", "基于 UE5 独立完成客户端全流程开发，支持本地双人对战"),
    ]
    for i, (k, v) in enumerate(cards):
        y = 140 + i * 110
        add_rect(s, 60, y, 1160, 90, CARD, BORDER)
        add_rect(s, 60, y, 6, 90, PRIMARY)
        add_text(s, 100, y + 18, 200, 28, k, size=16, color=PRIMARY, bold=True)
        add_text(s, 100, y + 48, 1080, 30, v, size=20, color=TEXT)

    # ---- 5 Chapter 02 ----
    s = new_slide(prs)
    page_chrome(s, "02 技术亮点", 5)
    add_text(s, 80, 240, 200, 120, "02", size=96, color=RGBColor(0x1E, 0x29, 0x3B), bold=True)
    add_text(s, 280, 280, 900, 50, "技术亮点", size=40, color=TEXT, bold=True)
    add_text(s, 280, 350, 900, 40, "Mass / Lyra Experience / CommonUI+MVVM / 近战 Sweep 判定", size=18, color=SECONDARY)

    # ---- 6 Highlights overview ----
    s = new_slide(prs)
    page_chrome(s, "02 技术亮点", 6)
    title_with_bar(s, "技术亮点总览")
    highlights = [
        ("Mass 与 ActorComponent", "数据-表现分离：MassEntity 管数据，Actor 管表现"),
        ("GameMode 优化", "Experience + GameFeature 实现玩法模块化与按需加载"),
        ("系统架构", "CommonUI + LyraUI + UnLua + MVVM"),
        ("近战判定优化", "AnimNotifyState + Socket 帧间 Sweep Trace"),
    ]
    for i, (title, desc) in enumerate(highlights):
        col = i % 2
        row = i // 2
        x = 60 + col * 590
        y = 140 + row * 240
        add_rect(s, x, y, 560, 210, CARD, BORDER)
        add_rect(s, x, y, 560, 6, PRIMARY)
        add_text(s, x + 28, y + 40, 500, 40, f"0{i+1}  {title}", size=22, color=TEXT, bold=True)
        add_text(s, x + 28, y + 100, 500, 80, desc, size=18, color=SECONDARY)

    # ---- 7 Mass ----
    s = new_slide(prs)
    page_chrome(s, "02 技术亮点", 7)
    title_with_bar(s, "Mass 与 ActorComponent 结合")
    add_bullets(s, 60, 140, 1160, 480, [
        "采用「数据-表现分离」架构",
        "数据层：MassEntity 管理 Fragment / Processor",
        "表现层：Actor 及其组件负责动画、特效、交互",
        "绑定：通过 UMassAgentComponent 关联 Entity 与 Actor",
        "同步：Translator 将 Fragment 数据单向同步至 ActorComponent",
    ], size=20, spacing=16)
    add_rect(s, 60, 560, 1160, 80, CARD, BORDER)
    add_text(s, 90, 580, 1100, 40, "目标：逻辑数据权威在 Mass，表现层只消费同步结果，降低耦合", size=18, color=ACCENT, bold=True)

    # ---- 8 GameMode ----
    s = new_slide(prs)
    page_chrome(s, "02 技术亮点", 8)
    title_with_bar(s, "GameMode 优化")
    add_bullets(s, 60, 140, 1160, 420, [
        "基于 Lyra 框架",
        "用 Experience + GameFeature 替代传统 GameMode 硬编码",
        "玩法规则、PawnData、UI 注入等由 Experience 数据资产配置",
        "实现玩法模块化与按需加载",
    ], size=20, spacing=18)
    add_rect(s, 60, 500, 1160, 120, CARD, BORDER)
    add_text(s, 90, 520, 1100, 30, "对比", size=16, color=PRIMARY, bold=True)
    add_text(s, 90, 555, 1100, 40, "传统：规则写死在 GameMode 子类  →  现在：Experience 数据驱动 + GameFeature 插件化加载", size=18, color=TEXT)

    # ---- 9 UI System ----
    s = new_slide(prs)
    page_chrome(s, "02 技术亮点", 9)
    title_with_bar(s, "系统架构（UI）")
    add_bullets(s, 60, 140, 1160, 360, [
        "CommonUI + LyraUI 分层管理菜单 / HUD / Modal",
        "结合 UnLua 与蓝图实现业务页面与局内 HUD",
        "角色属性 HUD 采用 MVVM 架构（ViewModel + FieldNotify）",
    ], size=20, spacing=18)
    layers = [("Menu 层", "主菜单 / 结算"), ("HUD 层", "血条 / 能量"), ("Modal 层", "确认 / 提示")]
    for i, (t, d) in enumerate(layers):
        x = 60 + i * 390
        add_rect(s, x, 430, 360, 160, CARD, BORDER)
        add_text(s, x + 24, 460, 300, 36, t, size=20, color=PRIMARY, bold=True)
        add_text(s, x + 24, 520, 300, 40, d, size=18, color=TEXT)

    # ---- 10 Melee ----
    s = new_slide(prs)
    page_chrome(s, "02 技术亮点", 10)
    title_with_bar(s, "近战判定优化")
    add_bullets(s, 60, 140, 1160, 360, [
        "基于 AnimNotifyState 定义攻击检测窗口",
        "每帧记录武器 Socket 上一帧与当前帧位置",
        "两点之间做 Sweep Trace，覆盖帧间扫过的区域",
        "解决高速攻击时帧间位移过大导致的漏检",
        "命中去重，避免同一次挥击重复结算",
    ], size=20, spacing=14)
    add_rect(s, 60, 540, 1160, 90, CARD, BORDER)
    add_text(s, 90, 565, 1100, 40, "PrevSocket → CurrSocket  Sweep Trace  +  HitActorsThisWindow 去重", size=18, color=ACCENT, bold=True)

    # ---- 11 Chapter 03 ----
    s = new_slide(prs)
    page_chrome(s, "03 架构细节", 11)
    add_text(s, 80, 240, 200, 120, "03", size=96, color=RGBColor(0x1E, 0x29, 0x3B), bold=True)
    add_text(s, 280, 280, 900, 50, "架构细节", size=40, color=TEXT, bold=True)
    add_text(s, 280, 350, 900, 40, "战斗组件、Experience 加载与 UI 数据流", size=20, color=SECONDARY)

    # ---- 12 Data flow ----
    s = new_slide(prs)
    page_chrome(s, "03 架构细节", 12)
    title_with_bar(s, "战斗与对局数据流")
    flows = [
        ("1. 输入开窗", "输入 → Combo / Combat 组件 → AnimNotifyState 开窗"),
        ("2. 命中结算", "Sweep Trace 命中 → Attribute 扣血 → HUD ViewModel 刷新"),
        ("3. 对局结束", "死亡事件 → GameMode / Experience 结算 → CommonUI 结算页"),
    ]
    for i, (title, desc) in enumerate(flows):
        y = 140 + i * 150
        add_rect(s, 60, y, 1160, 120, CARD, BORDER)
        add_rect(s, 60, y, 8, 120, PRIMARY if i < 2 else ACCENT)
        add_text(s, 100, y + 25, 1080, 30, title, size=20, color=PRIMARY, bold=True)
        add_text(s, 100, y + 65, 1080, 35, desc, size=18, color=TEXT)

    # ---- 13 Chapter 04 ----
    s = new_slide(prs)
    page_chrome(s, "04 总结与展望", 13)
    add_text(s, 80, 240, 200, 120, "04", size=96, color=RGBColor(0x1E, 0x29, 0x3B), bold=True)
    add_text(s, 280, 280, 900, 50, "总结与展望", size=40, color=TEXT, bold=True)
    add_text(s, 280, 350, 900, 40, "技术收获与后续方向", size=20, color=SECONDARY)

    # ---- 14 Summary ----
    s = new_slide(prs)
    page_chrome(s, "04 总结与展望", 14)
    title_with_bar(s, "总结")
    add_bullets(s, 60, 140, 1160, 280, [
        "完成 UE5 本地双人格斗客户端全流程",
        "实践 Mass 数据-表现分离、Lyra Experience 模块化、CommonUI/MVVM UI、近战 Sweep 判定",
        "后续：联机对战、AI 对手、更完整的 Mass Representation LOD",
    ], size=20, spacing=18)
    tags = ["Mass Entity", "Lyra Experience", "CommonUI / MVVM", "Sweep Trace"]
    for i, tag in enumerate(tags):
        x = 60 + i * 295
        add_rect(s, x, 500, 275, 80, CARD, BORDER)
        add_text(s, x + 10, 520, 255, 40, tag, size=16, color=PRIMARY, bold=True, align=PP_ALIGN.CENTER)

    # ---- 15 Final ----
    s = new_slide(prs)
    add_rect(s, 0, 310, 1280, 2, PRIMARY)
    add_text(s, 60, 250, 1160, 50, "感谢聆听", size=44, color=TEXT, bold=True, align=PP_ALIGN.CENTER)
    add_text(s, 60, 360, 1160, 30, "BlueBattle | UE5 双人格斗 | 欢迎交流", size=20, color=SECONDARY, align=PP_ALIGN.CENTER)
    add_text(s, 60, 520, 1160, 24, "https://github.com/www11223344/BlueBattle", size=14, color=PRIMARY, align=PP_ALIGN.CENTER)

    prs.save(OUT)
    print(f"Saved {OUT} ({len(prs.slides)} slides)")


if __name__ == "__main__":
    build()
