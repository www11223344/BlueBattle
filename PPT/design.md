# Design Document

## 1. Profile Baseline Declaration

- **Profile selection**: `profiles/general.md` 作为基线，融合 `profiles/academic.md` 的信息密度指导
- **Selection rationale**: 本项目是游戏技术展示而非纯学术论文，需要兼具科技感的视觉冲击力与技术内容的严谨性。目标受众可能是技术评审、课程导师或同行开发者，因此既要有视觉吸引力，又要保证技术内容的专业呈现
- **Referenced dimensions**: 从 general 参考整体布局自由度与创意空间；从 academic 参考信息密度控制（每页聚焦一个核心点）、公式/图表的规范展示、以及内容层级清晰度
- **Deviation notes**: 
  - 采用深色背景（偏离 academic 的浅色背景推荐），以突出游戏/科技氛围
  - 允许使用更丰富的装饰元素（渐变、线条、几何图形）增强视觉冲击力
  - 不强制使用参考文献页（用户未要求），但保留技术引用的严谨性

## 2. Style Baseline Declaration

- **Style anchor selection**: 
  - **Cyberpunk / Tech Noir**: 参考其深色背景+霓虹光效的配色逻辑，但降低饱和度，使整体更加专业克制
  - **Swiss International Style**: 参考其严格的网格对齐、清晰的字体层级和留白控制，确保技术内容的高可读性
- **Referenced dimension explanation**: 
  - 从 Cyberpunk 参考：深色背景、高对比度文字、科技感的蓝色/青色作为主色调，营造游戏开发的硬核氛围
  - 从 Swiss Style 参考：严格的网格对齐、无衬线字体层级、信息模块化布局，确保技术架构图、公式等内容的清晰呈现

## 3. Style Details

### 3.1 Color Design Principles

- **整体色彩倾向**: Striking & bold（科技游戏展示场景），但保持专业克制
- **色温**: 冷色调为主，营造科技感和冰雪场景的沉浸式氛围
- **主色 (Primary)**: `#0EA5E9`（Sky Blue 500）——明亮的天空蓝，传递科技感与虚幻引擎的蓝色基因，避免了廉价的深蓝/青色
- **背景色 (Background)**: `#0F172A`（Slate 900）——极深蓝灰色，深邃且有质感，避免纯黑的空洞感
- **文字色 (Text)**: `#F8FAFC`（Slate 50）——近白色，确保在深色背景上的高可读性
- **次要色 (Secondary)**: `#64748B`（Slate 500）——中灰色，用于辅助文字、次要信息、分隔线
- **强调色 (Accent)**: `#F59E0B`（Amber 500）——琥珀色，用于关键数据、高亮标注、重要结论，与冷色调形成强烈对比，起到视觉锚点作用。使用极其克制，仅用于页面中最需要强调的信息
- **卡片/容器色**: `#1E293B`（Slate 800）——比背景稍浅的蓝灰色，用于内容卡片、文本框背景，建立微妙的层级

### 3.2 Font Usage Principles

- **标题字体**: `Liter, MiSans` —— Liter 的现代新怪诞风格干净理性，适合科技感标题；MiSans 提供清晰的中文支持。标题使用加粗（font-weight: bold），封面大标题可配合宽字间距（letter-spacing: 2-4px）增强设计感
- **正文字体**: `Liter, MiSans` —— 保证中英文混排时的统一性和高可读性
- **字号层级**:
  - 封面标题: 40-48px
  - 页面标题: 28-32px
  - 副标题/模块标题: 22-24px
  - 正文内容: 18-20px（严禁低于18px）
  - 辅助说明/注释: 14-16px
  - 导航/页码: 12-14px
- **特殊处理**: 英文标题可全大写（如 "TECH ARCHITECTURE"），增加国际化和专业感

### 3.3 Text Box and Container Styles

- **内容分隔**: 优先使用留白和字号差异建立层级；当需要物理分隔时，使用细直线（1px, `#334155`）或微妙的卡片容器
- **卡片样式**: 使用直角矩形（无圆角，以契合Swiss风格的严谨），填充 `#1E293B`，无边框或1px `#334155` 边框。卡片内部使用16-20px的内边距（通过 bounds 控制）
- **装饰元素**: 
  - 标题左侧可使用 4px 宽的主色竖条作为视觉锚点
  - 章节过渡页使用大字号半透明数字（如 "01"，opacity 0.1）作为背景装饰
  - 避免使用无意义的几何图形堆砌

### 3.4 Image Style

- **Icons**: 使用 solid 风格（fas:），主色或次要色填充。仅在技术架构图、功能列表等需要辅助理解的场景使用，保持克制
- **Tables**: 极简科技风。表头使用主色 `#0EA5E9` 填充+白色文字，数据行使用交替的 `#1E293B` 和 `#0F172A` 背景，边框使用 `#334155`
- **Charts**: 图表系列色优先使用主色 `#0EA5E9` 及其同色系（如 `#38BDF8`, `#7DD3FC`），需要对比时引入强调色 `#F59E0B`。去除多余的网格线和背景，保持极简
- **Illustrations**: 优先使用项目实际截图（冰雪场景、蓝图等）。架构图、算法原理图使用 PPTD 的 Shape + Text 组合绘制，线条使用主色或次要色，确保矢量级清晰度和统一的科技风格

## 4. Layout System

### 4.1 Global Layout Characteristics

- **页面尺寸**: 1280 x 720 (16:9)
- **页面边距**: 左右 60px，上下 50px。确保内容区域有充足的呼吸空间
- **统一页面元素**:
  - 顶部导航栏：高度 40px，位于页面顶部，左侧放置当前章节名，右侧放置页码。导航栏背景可使用透明或主色细线分隔
  - 底部信息栏：高度 20px，放置项目名 "双人对战小游戏" 或辅助说明，文字色 `#64748B`
  - 页面标题区域：固定在页面左上角或顶部居中，标题下方可伴随 2px 宽的主色短横线（宽度约 40px）作为装饰

### 4.2 Special Page Layouts

- **Cover Page**: Hero design。全尺寸深色背景，中央放置超大号项目标题（"双人对战小游戏"），副标题使用次要色。背景可叠加极微弱的几何网格或项目截图的暗化蒙版，营造沉浸感
- **Table of Contents**: 非对称双栏布局。左侧放置巨大的 "CONTENTS" 字样（竖排或横排，使用主色，半透明），右侧使用网格布局排列章节，每个章节配有编号和名称，编号使用主色
- **Chapter Pages**: 章节过渡页。左侧 1/3 区域放置巨大的半透明章节编号（如 "01"），右侧 2/3 区域放置章节标题和一句话描述。背景可使用与章节内容相关的暗化图片或纯色渐变
- **Final Page**: 与封面呼应。中央放置 "感谢聆听"，下方放置项目名和联系方式/邀请讨论语

### 4.3 Content Page Layout Patterns

- **Pattern A (Single Column)**: 顶部标题 + 下方全宽内容区。适用于架构图、大段文字说明、公式展示。内容区在垂直方向上均匀分布
- **Pattern B (Left-Right Split)**: 左侧 50% 文字说明 + 右侧 50% 图表/截图。适用于算法原理讲解（左文字右公式图解）、功能对比（左文字右效果图）。左右两侧内容在垂直方向上保持视觉平衡（底部对齐）
- **Pattern C (Cards Grid)**: 顶部标题 + 下方 2-3 列等宽卡片。适用于功能介绍、系统模块详解。每个卡片内部使用图标+标题+描述的垂直结构
- **Pattern D (Top-Bottom)**: 顶部标题 + 中间图表 + 底部洞察/结论。适用于渲染效果对比、数据展示页。底部结论使用强调色高亮

## 5. Style Usage Rules

- **textStyle "title"**: 用于页面主标题、章节标题。使用主色或白色，加粗，最大字号
- **textStyle "subtitle"**: 用于模块标题、卡片标题。使用白色或主色，加粗，中等字号
- **textStyle "body"**: 用于正文内容、描述文字。使用文字色，常规字重，18-20px，行高 1.6
- **textStyle "caption"**: 用于图表注释、页码、辅助说明。使用次要色，14-16px
- **textStyle "accent"**: 用于关键数据、核心结论、高亮关键词。使用强调色 `#F59E0B`，加粗，仅在极少量内容使用
- **color "$primary"**: 用于标题装饰线、图标填充、表头背景、图表主系列色、链接色
- **color "$secondary"**: 用于辅助文字、次要分隔线、图表次要系列色
- **color "$accent"**: 用于关键数据高亮、重要结论、CTA元素
- **color "$background"**: 用于页面背景、数据行交替背景（可选）
- **color "$card"**: 用于内容卡片背景、文本框背景（当需要与页面背景区分时）
- **tableStyle "default"**: 用于所有数据表格。表头主色填充+白色文字，数据行交替使用 `$card` 和 `$background`，边框使用 `#334155`

## 6. Risk Prohibitions

- [ ] **禁止使用蓝/青色作为accent色**：本设计已选定琥珀色 `#F59E0B` 作为accent，严禁与主色混用导致对比度不足
- [ ] **禁止正文字号低于 18px**：在 720p 的投影环境下，低于 18px 的文字将难以辨认，影响技术内容传达
- [ ] **禁止卡片使用圆角**：Swiss/Tech 风格要求直角，圆角会削弱科技感与严谨性
- [ ] **禁止左右布局高度失衡**：左右分栏时，若一侧内容多一侧内容少，必须通过调整间距、增加辅助元素或缩小内容区域来保持视觉平衡，严禁出现一侧满、一侧大面积留白的情况
- [ ] **禁止纯白背景**：纯白背景与本设计的深色科技主题冲突，且会导致屏幕眩光
- [ ] **禁止在深色背景上使用深色文字**：所有文字必须在深色背景上保持高对比度（至少 WCAG AA 标准）
- [ ] **禁止图表使用默认配色**：图表必须使用主题定义的主色/次要色/强调色，禁止出现红绿等默认且冲突的配色
- [ ] **禁止过度使用强调色**：强调色 `#F59E0B` 仅用于每页最核心的1-2个数据或结论，大面积使用会导致视觉疲劳和焦点分散
- [ ] **禁止在架构图中使用过多色彩**：架构图、流程图应使用单色线条（主色或白色）+ 灰色/深色节点，仅在关键数据流或核心模块使用强调色

## 7. Theme Definition

```yaml
theme:
  colors:
    primary: "#0EA5E9"
    secondary: "#64748B"
    accent: "#F59E0B"
    background: "#0F172A"
    text: "#F8FAFC"
    card: "#1E293B"
    border: "#334155"
  textStyles:
    title:
      fontSize: 32
      color: "$text"
      fontFamily: "Liter, MiSans"
      fontWeight: "bold"
      lineHeight: 1.2
    subtitle:
      fontSize: 22
      color: "$primary"
      fontFamily: "Liter, MiSans"
      fontWeight: "bold"
      lineHeight: 1.3
    body:
      fontSize: 18
      color: "$text"
      fontFamily: "Liter, MiSans"
      lineHeight: 1.6
    caption:
      fontSize: 14
      color: "$secondary"
      fontFamily: "Liter, MiSans"
      lineHeight: 1.4
    accent:
      fontSize: 20
      color: "$accent"
      fontFamily: "Liter, MiSans"
      fontWeight: "bold"
      lineHeight: 1.4
  tableStyles:
    default:
      fontSize: 16
      fontFamily: "Liter, MiSans"
      headerFill: "$primary"
      headerColor: "#FFFFFF"
      headerBold: true
      bodyFill: ["$card", "$background"]
      bodyColor: "$text"
      border:
        style: solid
        width: 1
        color: "$border"
```