from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.enum.shapes import MSO_SHAPE

def add_textbox(slide, left, top, width, height, text, font_size=18, font_color=None, bold=False, align=PP_ALIGN.LEFT):
    shape = slide.shapes.add_textbox(left, top, width, height)
    tf = shape.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = text
    p.font.size = Pt(font_size)
    p.font.bold = bold
    p.font.name = "Microsoft YaHei"
    if font_color:
        p.font.color.rgb = RGBColor(font_color[0], font_color[1], font_color[2])
    p.alignment = align
    return shape

def add_content_with_bullets(slide, left, top, width, height, title, bullets, title_size=22, bullet_size=16):
    shape = slide.shapes.add_textbox(left, top, width, height)
    tf = shape.text_frame
    tf.word_wrap = True
    
    p = tf.paragraphs[0]
    p.text = title
    p.font.size = Pt(title_size)
    p.font.bold = True
    p.font.name = "Microsoft YaHei"
    p.font.color.rgb = RGBColor(14, 165, 233)
    
    for bullet in bullets:
        p = tf.add_paragraph()
        p.text = "  " + bullet
        p.font.size = Pt(bullet_size)
        p.font.name = "Microsoft YaHei"
        p.font.color.rgb = RGBColor(248, 250, 252)
        p.space_after = Pt(8)
    
    return shape

def main(ctx):
    pptx_path = r"C:\Users\zth05\xwechat_files\wxid_otg52aqtn4ye32_bfa7\msg\file\2026-06\虚幻引擎双人对打游戏开发.pptx"
    output_path = r"C:\杂项\虚幻b站项目\拳皇而已\双人对战小游戏\PPT\虚幻引擎双人对打游戏开发_修改版.pptx"
    
    prs = Presentation(pptx_path)
    
    blank_layout = prs.slide_layouts[0]
    sldIdLst = prs.slides._sldIdLst
    
    # 添加3个新幻灯片到末尾
    slide_anim = prs.slides.add_slide(blank_layout)
    slide_raycast = prs.slides.add_slide(blank_layout)
    slide_controlrig = prs.slides.add_slide(blank_layout)
    
    # 获取新添加的3个幻灯片的sldId XML元素
    all_sldIds = list(sldIdLst)
    new_sldIds = all_sldIds[-3:]
    
    # 移除它们
    for sldId in new_sldIds:
        sldIdLst.remove(sldId)
    
    # 正确插入位置（追踪最终索引）:
    # 动画通知在ComboSystem后: 索引9
    # 射线检测在双玩家控制后: 索引12
    # Control Rig在章节4(04)后: 索引17
    
    # 按逆序插入，避免索引偏移影响
    sldIdLst.insert(17, new_sldIds[2])  # Control Rig -> 最终索引17
    sldIdLst.insert(12, new_sldIds[1])  # 射线检测 -> 最终索引12
    sldIdLst.insert(9, new_sldIds[0])   # 动画通知 -> 最终索引9
    
    # 验证索引追踪:
    # 插入后: [0,1,2,3,4,5,6,7,8,anim,9,10,11,raycast,12,13,14,15,16,controlrig,17]
    # 索引:   0 1 2 3 4 5 6 7 8  9  10 11 12  13    14 15 16 17 18  19      20
    # 共20页，索引0-19
    # 索引9 = 动画通知 ✓
    # 索引12 = 双玩家控制 (原始11)，索引13 = 射线检测 ✓
    # 索引18 = 04 (原始14)，索引19 = Control Rig ✓
    
    # ===== 填充新幻灯片内容 =====
    
    # Slide 10: 动画通知与连击控制 (索引9)
    slide_anim = prs.slides[9]
    add_textbox(slide_anim, Inches(0.5), Inches(0.4), Inches(12), Inches(0.8), 
                "Animation Notify & Combo Control", font_size=28, font_color=(14, 165, 233), bold=True, align=PP_ALIGN.LEFT)
    add_textbox(slide_anim, Inches(0.5), Inches(0.9), Inches(12), Inches(0.5), 
                "动画通知与连击控制系统", font_size=20, font_color=(14, 165, 233), bold=True, align=PP_ALIGN.LEFT)
    
    line = slide_anim.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.5), Inches(1.35), Inches(2), Pt(2))
    line.fill.solid()
    line.fill.fore_color.rgb = RGBColor(14, 165, 233)
    line.line.color.rgb = RGBColor(14, 165, 233)
    
    left_bullets = [
        "AnimNotify机制: 在动画时间轴设置通知点(AttackStart/AttackEnd),精确控制攻击判定开关",
        "Combo Window: 攻击动画特定时间段内可输入下一次指令形成连击",
        "bIsAttacking: 布尔变量标记攻击状态,防止动画重复触发和无限连击",
        "Input Buffer: 攻击期间缓存输入,连击窗口开启时自动释放提升手感",
        "ComboCounter: 记录连击段数,驱动不同段数的攻击动画和伤害倍率"
    ]
    add_content_with_bullets(slide_anim, Inches(0.5), Inches(1.6), Inches(6), Inches(5), "Core Mechanism", left_bullets)
    
    right_bullets = [
        "AnimNotifyState -> NotifyBegin -> Set bIsAttacking=true -> Enable HitBox",
        "NotifyEnd -> Set bIsAttacking=false -> Disable HitBox -> Check ComboQueue",
        "ComboQueue缓存: 窗口期内检测输入键,缓存到队列等待窗口开启",
        "窗口期结束: ComboWindow Notify触发,检查队列并触发下一段攻击",
        "状态重置: 连击中断或达最大段数时,Counter清零等待新输入"
    ]
    add_content_with_bullets(slide_anim, Inches(7), Inches(1.6), Inches(5.5), Inches(5), "Implementation Flow", right_bullets)
    
    add_textbox(slide_anim, Inches(0.5), Inches(6.5), Inches(12), Inches(0.3), 
                "01 Project Overview | 02 Tech Architecture | 03 Core Algorithms | 04 Rendering | 05 Animation & Interaction", 
                font_size=12, font_color=(100, 116, 139), align=PP_ALIGN.LEFT)
    add_textbox(slide_anim, Inches(11.5), Inches(6.5), Inches(1), Inches(0.3), "10", 
                font_size=14, font_color=(100, 116, 139), align=PP_ALIGN.RIGHT)
    
    # Slide 14: 射线检测与判定系统 (索引13)
    slide_raycast = prs.slides[13]
    add_textbox(slide_raycast, Inches(0.5), Inches(0.4), Inches(12), Inches(0.8), 
                "Raycast & Hit Detection System", font_size=28, font_color=(14, 165, 233), bold=True, align=PP_ALIGN.LEFT)
    add_textbox(slide_raycast, Inches(0.5), Inches(0.9), Inches(12), Inches(0.5), 
                "射线检测与命中判定系统", font_size=20, font_color=(14, 165, 233), bold=True, align=PP_ALIGN.LEFT)
    
    line = slide_raycast.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.5), Inches(1.35), Inches(2), Pt(2))
    line.fill.solid()
    line.fill.fore_color.rgb = RGBColor(14, 165, 233)
    line.line.color.rgb = RGBColor(14, 165, 233)
    
    left_bullets = [
        "Line Trace / Raycast: 从武器Socket发射射线检测是否命中目标",
        "Channel Trace: 使用ECC_GameTraceChannel自定义攻击通道区分普通碰撞",
        "FHitResult: 获取命中位置、法线、受击组件、受击Actor等详细信息",
        "ApplyDamage: 命中后调用伤害函数,通过DamageEvent事件链通知受击方",
        "Debug Visualization: DrawDebugLine/Sphere可视化射线轨迹辅助调试"
    ]
    add_content_with_bullets(slide_raycast, Inches(0.5), Inches(1.6), Inches(6), Inches(5), "Raycast Mechanism", left_bullets)
    
    right_bullets = [
        "发射位置: 从武器Socket或骨骼插槽获取世界位置作为射线起点",
        "射线方向: 根据角色朝向GetActorForwardVector加攻击角度偏移",
        "检测范围: 设置合理的射线长度AttackRange和碰撞半径SphereTrace",
        "多目标处理: MultiLineTrace检测多个目标,按距离排序选择最近命中",
        "性能优化: 异步射线检测Async Line Trace避免阻塞主线程"
    ]
    add_content_with_bullets(slide_raycast, Inches(7), Inches(1.6), Inches(5.5), Inches(5), "Implementation Details", right_bullets)
    
    add_textbox(slide_raycast, Inches(0.5), Inches(6.5), Inches(12), Inches(0.3), 
                "01 Project Overview | 02 Tech Architecture | 03 Core Algorithms | 04 Rendering | 05 Animation & Interaction", 
                font_size=12, font_color=(100, 116, 139), align=PP_ALIGN.LEFT)
    add_textbox(slide_raycast, Inches(11.5), Inches(6.5), Inches(1), Inches(0.3), "14", 
                font_size=14, font_color=(100, 116, 139), align=PP_ALIGN.RIGHT)
    
    # Slide 20: Control Rig受伤动画修正 (索引19)
    slide_controlrig = prs.slides[19]
    add_textbox(slide_controlrig, Inches(0.5), Inches(0.4), Inches(12), Inches(0.8), 
                "Control Rig Hit Reaction Correction", font_size=28, font_color=(14, 165, 233), bold=True, align=PP_ALIGN.LEFT)
    add_textbox(slide_controlrig, Inches(0.5), Inches(0.9), Inches(12), Inches(0.5), 
                "Control Rig 受伤动画实时修正", font_size=20, font_color=(14, 165, 233), bold=True, align=PP_ALIGN.LEFT)
    
    line = slide_controlrig.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.5), Inches(1.35), Inches(2), Pt(2))
    line.fill.solid()
    line.fill.fore_color.rgb = RGBColor(14, 165, 233)
    line.line.color.rgb = RGBColor(14, 165, 233)
    
    left_bullets = [
        "Control Rig: UE5实时动画控制系统,运行时通过控制点动态调整骨骼变换",
        "Hit Reaction Problem: 传统受击动画朝向可能与攻击来源方向不匹配",
        "Direction Correction: 获取DamageCauser位置,计算攻击方向向量",
        "Dynamic Orientation: 根据攻击方向实时调整Root或Spine骨骼旋转",
        "Physical Feedback: 结合Physical Animation,受击时驱动骨骼物理摆动"
    ]
    add_content_with_bullets(slide_controlrig, Inches(0.5), Inches(1.6), Inches(6), Inches(5), "Control Rig Solution", left_bullets)
    
    right_bullets = [
        "Rig Graph: 创建受伤修正Rig,添加Bone节点和Transform控制点",
        "Angle Calculation: LookAtRotation指向攻击来源,限制最大修正角度60度",
        "Smooth Transition: InterpTo/Spring插值避免修正跳变,实现自然转向",
        "Blend Weight: 动画蓝图中设置Control Rig权重与原始动画混合",
        "Multi-tier Adaptation: 轻击仅上半身晃动,重击全身击退+倒地,不同Rig强度"
    ]
    add_content_with_bullets(slide_controlrig, Inches(7), Inches(1.6), Inches(5.5), Inches(5), "Technical Details", right_bullets)
    
    add_textbox(slide_controlrig, Inches(0.5), Inches(6.5), Inches(12), Inches(0.3), 
                "01 Project Overview | 02 Tech Architecture | 03 Core Algorithms | 04 Rendering | 05 Animation & Interaction", 
                font_size=12, font_color=(100, 116, 139), align=PP_ALIGN.LEFT)
    add_textbox(slide_controlrig, Inches(11.5), Inches(6.5), Inches(1), Inches(0.3), "20", 
                font_size=14, font_color=(100, 116, 139), align=PP_ALIGN.RIGHT)
    
    # 保存
    prs.save(output_path)
    
    return {"output_path": output_path, "total_slides": len(prs.slides)}
