# Presentation Outline

## Page 1 [cover]
- **Title**: 双人对战小游戏
- **Content**: 基于虚幻引擎5的格斗游戏项目展示 | 技术架构与核心算法解析

## Page 2 [table_of_contents]
- **Title**: 目录
- **Content**: 1. 项目概述; 2. 技术架构; 3. 核心算法; 4. 渲染效果; 5. 动画与交互; 6. 学习体会

## Page 3 [chapter]
- **Title**: 01 项目概述
- **Content**: 基于虚幻引擎5的双人格斗游戏设计与实现

## Page 4 [content]
- **Title**: 项目功能介绍
- **Content**: 
  - 双人对战模式：支持两名玩家本地实时对战，采用经典格斗游戏操作逻辑
  - 拳皇模式（KOF Mode）：致敬经典格斗游戏，包含连招、必杀技等核心机制
  - 角色系统（BP_Warrior）：完整的角色蓝图，包含移动、攻击、受击、死亡等状态机
  - 属性系统（AttributeSystem）：生命值、攻击力、防御力等属性管理，支持属性实时变化
  - 连招系统（ComboSystem）：支持多段连击，判定窗口精确，打击感反馈强烈
  - 经典移动系统（ClassicStrafeMovement）：8方向移动、冲刺、闪避等操作

## Page 5 [content]
- **Title**: 游戏场景与角色展示
- **Content**: 
  - 冰雪对战场景（ice.umap）：雪山松林环境，使用虚幻引擎5 Nanite虚拟化几何和Lumen全局光照
  - 拳皇角色蓝图：自定义角色模型与动画，包含待机、移动、攻击、受击动画状态
  - 实时渲染效果：动态光照、粒子特效、材质反射

## Page 6 [chapter]
- **Title**: 02 技术架构
- **Content**: 系统分层设计与模块交互

## Page 7 [content]
- **Title**: 技术架构图
- **Content**: 
  - 展示四层架构：引擎层（UE5引擎核心）→ 框架层（Gameplay Framework）→ 系统层（战斗/输入/动画）→ 表现层（渲染/UI/音效）
  - 核心模块：玩家控制器、游戏模式、角色蓝图、动画蓝图、输入系统、战斗系统、状态机
  - 数据流向：输入 → 玩家控制器 → 角色 → 动画 → 渲染

## Page 8 [content]
- **Title**: 核心系统模块详解
- **Content**: 
  - 输入系统：虚幻引擎增强输入系统（Enhanced Input），支持键鼠映射，动作/轴绑定
  - 战斗系统：攻击判定（HitBox）、伤害计算（Damage Formula）、受击反馈（Hit Reaction）
  - 动画系统：动画蓝图状态机（AnimStateMachine），混合空间（BlendSpace），根运动（RootMotion）
  - 网络架构：本地双人分屏，支持输入同步与状态回滚

## Page 9 [content]
- **Title**: 射线检测与判定系统
- **Content**: 
  - 射线检测（Line Trace / Raycast）：从攻击方发射射线检测碰撞，实现精确命中判定
  - 判定类型：Channel Trace（通道检测）vs Object Trace（对象检测），使用ECC_GameTraceChannel自定义攻击通道
  - 命中响应：检测到命中后，获取命中结果（FHitResult），提取命中位置、法线、受击组件
  - 伤害传递：通过射线检测命中目标后，调用ApplyDamage函数，触发DamageEvent事件链
  - 调试可视化：使用DrawDebugLine和DrawDebugSphere在编辑器中可视化射线轨迹，辅助调试判定范围

## Page 10 [chapter]
- **Title**: 03 核心算法
- **Content**: 光照模型与加速结构

## Page 11 [content]
- **Title**: Phong光照模型
- **Content**: 
  - Phong光照公式：$I_o = k_a I_a + I_i [k_d \max(0, N \cdot L) + k_s \max(0, V \cdot R)^n]$
  - 环境光（Ambient）：模拟间接光照，提供基础亮度 $I_a$
  - 漫反射（Diffuse）：Lambert余弦定律，与表面法线和光线方向相关 $k_d \max(0, N \cdot L)$
  - 镜面反射（Specular）：高光效果，取决于观察方向与反射方向夹角 $k_s \max(0, V \cdot R)^n$
  - 在虚幻引擎中的实现：通过材质编辑器（Material Editor）和自定义HLSL Shader实现Phong光照

## Page 12 [content]
- **Title**: BVH加速结构
- **Content**: 
  - BVH（Bounding Volume Hierarchy）层次包围盒：基于图元分割的射线求交加速结构
  - 构建过程：将场景图元按空间邻近性分组，构建二叉树层次结构
  - 遍历算法：从根节点开始，射线与包围盒求交，若未相交则剪枝整个子树
  - 复杂度分析：从朴素算法的 $O(In)$ 优化到平均 $O(I \log n)$，$I$ 为射线数，$n$ 为图元数
  - 在虚幻引擎中的应用：Lumen全局光照和光线追踪（Ray Tracing）使用BVH加速场景遍历

## Page 13 [chapter]
- **Title**: 04 渲染效果
- **Content**: 视觉效果对比与性能优化

## Page 14 [content]
- **Title**: 渲染效果对比
- **Content**: 
  - 光照对比：Flat Shading vs Gouraud Shading vs Phong Shading 效果对比
  - BVH优化对比：有无BVH加速结构的渲染时间对比（帧率/耗时数据）
  - 虚幻引擎5特性：Nanite虚拟化几何、Lumen动态全局光照、实时反射
  - 场景渲染效果：冰雪场景的光照、材质、阴影效果展示

## Page 15 [chapter]
- **Title**: 05 动画与交互
- **Content**: 连招系统与动画修正技术

## Page 16 [content]
- **Title**: 连击控制与动画通知
- **Content**: 
  - 连击窗口（Combo Window）：在攻击动画的特定时间段内，玩家可以输入下一次攻击指令，形成连击
  - 动画通知（AnimNotify）：在动画时间轴上设置通知点（AttackStart/AttackEnd），精确控制攻击判定开关
  - 攻击状态管理：使用布尔变量bIsAttacking标记角色是否处于攻击状态，防止动画重复触发和无限连击
  - 输入缓冲（Input Buffer）：在攻击动画期间缓存玩家输入，在连击窗口开启时自动释放，提升操作手感
  - 连击计数器：ComboCounter记录当前连击段数，驱动不同段数的攻击动画和伤害倍率

## Page 17 [content]
- **Title**: Control Rig 受伤动画修正
- **Content**: 
  - Control Rig简介：虚幻引擎5的实时动画控制系统，支持在运行时动态调整骨骼变换
  - 受伤动画问题：传统受击动画播放时，角色可能与攻击来源方向不匹配，导致"穿帮"
  - 方向修正：使用Control Rig在受击动画播放时，实时获取攻击来源方向（DamageCauser），动态调整角色朝向
  - 物理反馈：结合物理动画（Physics Animation），在受击时根据冲击力方向驱动局部骨骼摆动
  - 混合修正：Control Rig修正与动画蓝图的Blend节点结合，实现平滑过渡，避免突兀的转向跳变

## Page 18 [chapter]
- **Title**: 06 学习体会
- **Content**: 技术成长与项目反思

## Page 19 [content]
- **Title**: 技术收获与心得
- **Content**: 
  - 深入理解了虚幻引擎5的Gameplay Framework架构，掌握了蓝图与C++的混合开发模式
  - 通过学习Phong光照模型和BVH加速结构，加深了对计算机图形学底层原理的理解
  - 体会到了游戏开发的系统性思维：从游戏设计、系统架构到性能优化的完整链路
  - 认识到引擎封装与底层原理的结合是高效开发的关键
  - 未来展望：引入网络多人对战、AI对手系统、更复杂的物理交互

## Page 20 [final]
- **Title**: 感谢聆听
- **Content**: 双人对战小游戏 | 虚幻引擎5项目 | 欢迎交流与讨论
