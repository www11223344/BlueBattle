# BlueBattle · C++ 战斗框架架构说明

> 本文档面向代码评审 / 简历讲解，说明本项目从「纯蓝图逻辑」演进为「Mass 数据层 + Actor 表现层」的架构设计、模块职责与关键取舍。

## 0. 技术亮点（简历对齐）

基于 UE5 独立完成客户端全流程开发，支持本地双人对战。

- **Mass 与 ActorComponent 结合**：采用「数据-表现分离」架构，数据层由 MassEntity 管理，表现层由 Actor 及其组件负责，通过 `UMassAgentComponent` 绑定，由 Translator 将 Fragment 数据单向同步至 ActorComponent。
- **GameMode 优化**：基于 Lyra 框架，用 Experience + GameFeature 替代传统 GameMode 硬编码，实现玩法模块化与按需加载。
- **系统架构**：采用 CommonUI + LyraUI，结合 UnLua 与蓝图实现业务页面与局内 HUD；角色属性 HUD 采用 MVVM 架构。
- **近战判定优化**：基于 AnimNotifyState 定义攻击检测窗口，每帧记录武器 Socket 的上一帧与当前帧位置做 Sweep Trace，解决高速攻击时帧间位移过大导致的漏检，并做命中去重避免同一次挥击重复结算。

## 1. 一句话概述

每个角色的**权威数据与玩法规则**（属性、连招 / 闪避 / 爆气状态机、招式表、命中结算）都放在一个 **MassEntity** 上，由 Processor 推进、由写入子系统修改；Actor 及其组件只是**只读镜像 + 表现**，由 Translator 每帧把 Fragment 单向同步过来，状态变化时以委托 / `BlueprintImplementableEvent` 通知蓝图播放蒙太奇、特效、音效和刷新 UI。

```
            输入 / 碰撞上报                       单向同步（Translator）
 蓝图 / 组件 ───────────────▶ UBlueBattleUnitSubsystem ──▶ Mass Fragment ──────────────▶ ActorComponent 镜像 ──▶ 委托 / 蓝图事件
 （表现层）     （BlueprintCallable）   （唯一写入通道）     （权威数据）  ◀── Processor 推进                （只读）          （表现）
```

## 2. 模块总览

### 2.1 数据层（`Mass/`）

| 模块 | 文件 | 职责 |
| --- | --- | --- |
| Fragment / Tag | `BlueBattleMassFragments.h` | 属性、连招状态、招式表、配置参数等全部权威数据 |
| Trait | `BlueBattleMassTraits.h/.cpp` | 组装实体模板（Fragment + Translator），实体创建时从组件导出配置初值并绑定实体句柄 |
| 绑定组件 | `BlueBattleMassAgentComponent.h/.cpp` | `UMassAgentComponent` 子类，挂在角色上，注册时创建实体，绑定完成后广播 `OnEntityChanged` |
| 写入子系统 | `BlueBattleUnitSubsystem.h/.cpp` | 表现层修改数据的唯一入口：出招 / 闪避 / 爆气判定、命中结算、扣血 / 耗魔 / 能量增减 |
| 属性 Processor | `BlueBattleAttributeProcessors.h/.cpp` | MP 回复、能量衰减；HP 归零打死亡 Tag |
| 连招 Processor | `BlueBattleComboProcessors.h/.cpp` | 攻击 / 闪避互斥、闪避冷却倒计时；爆气倒计时并到时移除爆气 Tag |
| Translator | `BlueBattleAttributeTranslator.h/.cpp`、`BlueBattleComboTranslator.h/.cpp` | 游戏线程上把 Fragment 同步到 `UAttributeComponent` / `UComboComponent` |

### 2.2 表现层

| 模块 | 文件 | 职责 |
| --- | --- | --- |
| 角色基类 | `Characters/WarriorCharacter.h/.cpp` | 组合各组件，作为输入入口；死亡时通知 GameMode |
| 属性组件 | `Combat/AttributeComponent.h/.cpp` | 属性配置 + 运行时只读镜像，值变化时广播委托，受击 / 死亡时回调蓝图；写操作转发到子系统 |
| 连招组件 | `Combat/ComboComponent.h/.cpp` | 连招 / 爆气 / 闪避配置与招式表 + 状态镜像；出招成功回调蓝图播放；`ReportHit` 上报命中；爆气切换移速 |
| 战斗组件 | `Combat/CombatComponent.h/.cpp` | 可选的扫掠命中检测，命中经 `ReportHit` 交给数据层结算 |
| 动画通知 | `Combat/WarriorAnimNotifies.h/.cpp` | `Attack Window` / `Combo Window` 两个 `AnimNotifyState` |
| 属性 ViewModel | `UI/ViewModels/BlueBattleAttributeViewModel.h/.cpp` | MVVM：订阅属性组件委托，驱动头顶血条 / HUD |
| 对局模式 | `Core/BlueBattleGameMode.h/.cpp` | 监听死亡、判定胜负、广播 `OnMatchEnded` |

### 2.3 蓝图

| 资产 | 职责 |
| --- | --- |
| `BP_Warrior` | 组件装配；伤害 Box 碰撞后把对方交给 `ComboSystem.ReportHit`；AI 感知记录对方是否在身前。不含其他表现逻辑 |
| `ComboSystem`（继承 `UComboComponent`） | `OnPlayAttackMontage`（按 `bEX` 选播放速率与挥剑音效）、`OnPlayDodgeMontage`（按闪避方向选蒙太奇）；爆气期间 Tick 按速度生成残影 |
| `AttributeSystem`（继承 `UAttributeComponent`） | 属性配置；`OnDamagedFeedback`（按身前 / 身后播受击动作 + 击中音效）、`OnDeathFeedback`（关碰撞、播死亡动画后销毁角色） |
| `ABP_fab` | 攻击 / 闪避结束的动画通知调用 `EndAttack` / `EndDodge` |

表现逻辑全部落在组件上，由组件自己的回调 / Tick 驱动；Actor 图里不再轮询组件状态。

## 3. 设计原则

1. **单一数据源**：运行时数值与状态只存在于 Fragment 中。组件上的同名字段是 Translator 写入的镜像（`VisibleAnywhere` + `BlueprintReadOnly`），比 Fragment 最多晚一帧，蓝图无法改写。
2. **单向同步**：Mass → 组件只有 Translator 一条路径；组件 → Mass 只能经 `UBlueBattleUnitSubsystem` 的函数调用，表现层不直接写 Fragment。
3. **规则在数据层，表现在 Actor**：所有「决定发生什么」（能否出招、出哪一招、伤害多少、何时死亡、爆气多久）都在子系统 / Processor 中；所有「看起来 / 听起来如何」在蓝图中。
4. **配置在组件，导入到实体**：策划调参仍在组件 Details 面板（`AttributeSystem` / `ComboSystem`）完成；实体创建时由 Trait 初始化器调用 `ExportToMass` 拷入 Fragment，运行时以 Fragment 为准。
5. **变化驱动事件**：Translator 每帧同步，但组件只在值真正变化时才广播委托，UI / 特效不会被每帧触发。

## 4. 数据层结构

| Fragment / Tag | 内容 | 来源 |
| --- | --- | --- |
| `FBlueBattleAttributeFragment` | HP / MP / Energy 及上限、攻防、最近伤害来源 | `UAttributeComponent::ExportToMass` |
| `FBlueBattleAttributeRegenFragment` | MP 回复速度、能量衰减速度 | 同上 |
| `FBlueBattleComboFragment` | 连击计数、攻击 / 闪避 / 连招窗口状态、爆气剩余时间、闪避冷却、当前招式及其伤害 | 运行时 |
| `FBlueBattleComboParamsFragment` | 爆气时长 / 能量消耗、闪避冷却 | `UComboComponent::ExportToMass` |
| `FBlueBattleSkillSetFragment` | 轻 / 重 / EX 轻 / EX 重四组招式序列（以蒙太奇标识）与每招伤害表 | 同上 |
| `FBlueBattleDeadTag` / `FBlueBattleBurstTag` | 已死亡 / 爆气中 | 子系统、Processor |
| `FBlueBattle*ComponentWrapperFragment` | 指向对应组件的弱引用，供 Translator 写回 | Trait 初始化器 |

`FBlueBattleSkillSetFragment` 含 `TArray` / `TMap`，通过特化 `TMassFragmentTraits::AuthorAcceptsItsNotTriviallyCopyable` 显式声明为非平凡拷贝 Fragment（每个单位一份，数量极少，无性能顾虑）。

## 5. 关键数据流

### 5.1 实体绑定
```
AWarriorCharacter 构造 → MassAgentComponent（Unit / AttributeSync / ComboSync 三个 Trait）
  → OnRegister 向 MassAgentSubsystem 注册 → 下一次 Mass 处理阶段异步创建实体
    → Trait 初始化器：组件 ExportToMass 导出配置、BindMassEntity 记录句柄
    → SetEntityHandleInternal：日志 "bound to Mass entity"，广播 OnEntityChanged
```

### 5.2 出招
```
输入(轻击/重击) → WarriorCharacter::LightAttack/HeavyAttack → ComboComponent::PerformAttack
  → UnitSubsystem::TryStartAttack            // 存活、非攻击/闪避中；按类型+是否爆气取招式序列，
                                             // 第 (ComboIndex % 长度) 招，记录当前招式与伤害，计数+1
  → OnPlayAttackMontage(类型, bEX, 段数, 招式蒙太奇)   // 蓝图：播放 + 音效
攻击动画结束 → ABP_fab AnimNotify_Atc_END → ComboComponent::EndAttack → 子系统清攻击状态
```

### 5.3 命中结算
```
BP_Warrior 伤害 Box 开始重叠 → ComboSystem.ReportHit(对方)
  → UnitSubsystem::ResolveHit               // 攻击者存活且在攻击中、目标为其他存活单位、当前招式伤害>0
    → ApplyDamage(目标)                     // 扣除防御后扣血，HP 归零打死亡 Tag
    → AddEnergy(攻击者, 招式伤害)
目标的 UBlueBattleAttributeToActorTranslator 同步到 HP 下降
  → 目标 AttributeComponent::OnDamagedFeedback(伤害, 攻击者)   // 蓝图：受击动作 + 击中音效
（CombatComponent 的扫掠检测走同一个 ReportHit，可替换碰撞 Box）
```

### 5.4 闪避
```
输入(闪避) → WarriorCharacter::Dodge → ComboComponent::PerformDodge
  → UnitSubsystem::TryStartDodge            // 存活、非闪避中、冷却结束；打断攻击，进入冷却
  → OnPlayDodgeMontage → 蓝图按闪避方向选蒙太奇（找不到时调用 EndDodge）
闪避动画结束 → ABP_fab AnimNotify_EvasionEnd → EndDodge
UBlueBattleComboStateProcessor：攻击与闪避同时为真时都清除；闪避冷却倒计时
```

### 5.5 爆气 / EX
```
输入(爆气) → WarriorCharacter::Burst → ComboComponent::ActivateBurst
  → UnitSubsystem::TryActivateBurst         // 能量达到 EX 阈值且未在爆气中：扣能量、设剩余时间、加爆气 Tag
UBlueBattleBurstTimerProcessor              // 倒计时，到时移除爆气 Tag
UBlueBattleComboToActorTranslator → ComboComponent::ApplyMassSnapshot
  → 爆气状态变化：切换 MaxWalkSpeed、开关组件 Tick、广播 OnBurstStateChanged
  → ComboSystem Tick（仅爆气期间）：速度 ≥400 时生成残影
```

### 5.6 属性与死亡
```
UBlueBattleAttributeRegenProcessor          // MP 向上限回复、能量向 0 衰减（死亡后停止）
UBlueBattleDeathProcessor                   // HP<=0 打死亡 Tag
UBlueBattleAttributeToActorTranslator → AttributeComponent::ApplyMassSnapshot
  → OnHPChanged / OnMPChanged / OnEnergyValueChanged → ViewModel → 血条 / 蓝条 / 能量条
  → 首次同步到死亡：OnDeathFeedback（蓝图：关碰撞、死亡动画、销毁）
                   → OnDeath 广播 → WarriorCharacter::HandleDeath → GameMode 判负
```

## 6. 对外接口（C++ / 蓝图边界）

- **输入入口**：`AWarriorCharacter::LightAttack` / `HeavyAttack` / `Dodge` / `Burst` / `ConsumeMP`。
- **数据层写入（BlueprintCallable，均转发到子系统）**：
  - `UComboComponent`：`PerformLightAttack` / `PerformHeavyAttack` / `PerformDodge` / `ActivateBurst` / `EndAttack` / `EndDodge` / `ResetCombo` / `ReportHit` / `Open|CloseComboWindow`。
  - `UAttributeComponent`：`ApplyDamage` / `Heal` / `ConsumeMP` / `ConsumeEnergyValue` / `AddEnergyValue`。
- **表现层回调（组件蓝图实现）**：`UComboComponent::OnPlayAttackMontage` / `OnPlayDodgeMontage`；`UAttributeComponent::OnDamagedFeedback` / `OnDeathFeedback`。
- **状态查询（镜像，BlueprintPure）**：`IsAttacking` / `IsEvading` / `IsBursting` / `GetComboCount` / `IsAlive` 及属性组件的 HP / MP / EnergyValue / CanEX / IsDead。
- **事件订阅**：`OnHPChanged` / `OnMPChanged` / `OnEnergyValueChanged` / `OnDeath` / `OnComboStepChanged` / `OnComboReset` / `OnBurstStateChanged` / `OnHitTarget` / `OnMatchEnded`。

## 7. 迁移说明与取舍

- 原项目为纯蓝图实现。迁移分阶段完成：接入 Mass → 属性迁入 → 连招 / 爆气 / 闪避状态机迁入 → 招式表与命中结算迁入 → 清理蓝图与 C++ 中残留的规则。每阶段都保持可玩并经 PIE 验证。
- `BP_Warrior` 的 `AttributeSystem` / `ComboSystem` 组件 **Reparent** 到 C++ 基类，原蓝图变量（`Attacking?`、`ComboIndex`、`技能伤害表` 等）及对应逻辑已删除，数值迁到 C++ 配置字段，原值保持不变。
- **写入走子系统而不是 Processor**：出招 / 命中等由输入或碰撞触发、需要立即得到结果（如要播哪个蒙太奇、造成多少伤害），因此在游戏线程同步修改 Fragment；持续性规则（回复、衰减、倒计时、互斥）放在 Processor 中逐帧推进。子系统在 Mass 处理阶段内加减 Tag 时自动改走延迟命令。
- **招式以蒙太奇标识**：沿用原蓝图「蒙太奇 → 伤害」的配置方式，数据层只把蒙太奇当作招式 ID 使用，播放速率、音效仍由蓝图决定。
- `FWarriorAttributes` / `FComboMoveData`（`BlueBattleTypes.h`）为早期预留的 DataTable 结构，当前未接入。

## 8. 面试讲解要点

- **为什么用 Mass 承载单位数据**：数据与表现解耦后，规则代码只依赖 Fragment，不依赖 Actor / 蓝图；同一套 Processor 可以推进任意数量的单位，Actor 可以按需挂接或不挂接（如远景 / 服务器端只跑数据层）。
- **单向数据流**：写入只有子系统一个入口，读取只有 Translator 一个出口，任何数值问题都能沿这条链路定位，不会出现蓝图与 C++ 各改一份的情况。
- **Translator + 变化检测**：每帧同步保证镜像一致，变化时才广播委托，UI / 特效按事件驱动。
- **结构变更安全**：死亡 / 爆气用 Tag 表达，Processor 内用 `Context.Defer()`，处理阶段外的子系统调用自动判断是否需要延迟。
- **配置与运行时分离**：组件负责可编辑的配置，Trait 初始化器在实体创建时导入，运行时数据完全在 Fragment 中。
