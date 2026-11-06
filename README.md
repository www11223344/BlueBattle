# BlueBattle — 3D 双人格斗对战游戏

**独立开发** · 2026.05 - 2026.06  
项目链接：https://github.com/www11223344/BlueBattle

基于 UE5 独立完成客户端全流程开发，支持本地双人对战。

## 技术亮点

- **Mass 与 ActorComponent 结合**：采用「数据-表现分离」架构，数据层由 MassEntity 管理，表现层由 Actor 及其组件负责，通过 `UMassAgentComponent` 绑定，由 Translator 将 Fragment 数据单向同步至 ActorComponent。
- **GameMode 优化**：基于 Lyra 框架，用 Experience + GameFeature 替代传统 GameMode 硬编码，实现玩法模块化与按需加载。
- **系统架构**：采用 CommonUI + LyraUI，结合 UnLua 与蓝图实现业务页面与局内 HUD；角色属性 HUD 采用 MVVM 架构。
- **近战判定优化**：基于 AnimNotifyState 定义攻击检测窗口，每帧记录武器 Socket 的上一帧与当前帧位置做 Sweep Trace，解决高速攻击时帧间位移过大导致的漏检，并做命中去重避免同一次挥击重复结算。

## 架构说明

更细的 C++ 战斗框架说明见 [`Source/BlueBattle/ARCHITECTURE.md`](Source/BlueBattle/ARCHITECTURE.md)。
