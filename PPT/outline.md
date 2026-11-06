# Presentation Outline

## Page 1 [cover]
- **Title**: BlueBattle — 3D 双人格斗对战游戏
- **Content**: 基于虚幻引擎5的本地双人对战格斗项目 | 技术架构与核心实现

## Page 2 [table_of_contents]
- **Title**: 目录
- **Content**: 1. 项目概述; 2. 技术亮点; 3. 架构细节; 4. 总结与展望

## Page 3 [chapter]
- **Title**: 01 项目概述
- **Content**: 独立完成 UE5 客户端全流程开发，支持本地双人对战

## Page 4 [content]
- **Title**: 项目简介
- **Content**:
  - 项目：BlueBattle — 3D 双人格斗对战游戏
  - 角色：独立开发 | 周期：2026.05 - 2026.06
  - 链接：https://github.com/www11223344/BlueBattle
  - 基于 UE5 独立完成客户端全流程开发，支持本地双人对战

## Page 5 [chapter]
- **Title**: 02 技术亮点
- **Content**: Mass / Lyra Experience / CommonUI+MVVM / 近战 Sweep 判定

## Page 6 [content]
- **Title**: 技术亮点总览
- **Content**:
  - Mass 与 ActorComponent 结合：数据-表现分离
  - GameMode 优化：Experience + GameFeature 模块化玩法
  - 系统架构：CommonUI + LyraUI + UnLua + MVVM
  - 近战判定优化：AnimNotifyState + Socket 帧间 Sweep Trace

## Page 7 [content]
- **Title**: Mass 与 ActorComponent 结合
- **Content**:
  - 采用「数据-表现分离」架构
  - 数据层：MassEntity 管理 Fragment / Processor
  - 表现层：Actor 及其组件负责动画、特效、交互
  - 绑定：通过 UMassAgentComponent 关联 Entity 与 Actor
  - 同步：Translator 将 Fragment 数据单向同步至 ActorComponent

## Page 8 [content]
- **Title**: GameMode 优化
- **Content**:
  - 基于 Lyra 框架
  - 用 Experience + GameFeature 替代传统 GameMode 硬编码
  - 玩法规则、PawnData、UI 注入等由 Experience 数据资产配置
  - 实现玩法模块化与按需加载

## Page 9 [content]
- **Title**: 系统架构（UI）
- **Content**:
  - CommonUI + LyraUI 分层管理菜单 / HUD / Modal
  - 结合 UnLua 与蓝图实现业务页面与局内 HUD
  - 角色属性 HUD 采用 MVVM 架构（ViewModel + FieldNotify）

## Page 10 [content]
- **Title**: 近战判定优化
- **Content**:
  - 基于 AnimNotifyState 定义攻击检测窗口
  - 每帧记录武器 Socket 上一帧与当前帧位置
  - 两点之间做 Sweep Trace，覆盖帧间扫过的区域
  - 解决高速攻击时帧间位移过大导致的漏检
  - 命中去重，避免同一次挥击重复结算

## Page 11 [chapter]
- **Title**: 03 架构细节
- **Content**: 战斗组件、Experience 加载与 UI 数据流

## Page 12 [content]
- **Title**: 战斗与对局数据流
- **Content**:
  - 输入 → Combo / Combat 组件 → AnimNotifyState 开窗
  - Sweep Trace 命中 → Attribute 扣血 → HUD ViewModel 刷新
  - 死亡事件 → GameMode / Experience 结算 → CommonUI 结算页

## Page 13 [chapter]
- **Title**: 04 总结与展望
- **Content**: 技术收获与后续方向

## Page 14 [content]
- **Title**: 总结
- **Content**:
  - 完成 UE5 本地双人格斗客户端全流程
  - 实践 Mass 数据-表现分离、Lyra Experience 模块化、CommonUI/MVVM UI、近战 Sweep 判定
  - 后续：联机对战、AI 对手、更完整的 Mass Representation LOD

## Page 15 [final]
- **Title**: 感谢聆听
- **Content**: BlueBattle | UE5 双人格斗 | 欢迎交流
