# BlueBattle · C++ 战斗框架架构说明

> 本文档面向代码评审 / 简历讲解，说明本项目从「纯蓝图逻辑」向「C++ 核心 + 蓝图表现」演进的架构设计、模块职责与关键取舍。

## 1. 一句话概述

把角色的**核心玩法逻辑（属性、命中、连招/爆气状态机、对局判定）用 C++ 组件化实现**，蓝图只保留**表现层**（蒙太奇、特效、音效、UI、输入映射）。C++ 通过 `UPROPERTY / UFUNCTION / 委托(Delegate)` 暴露给蓝图，形成清晰的「逻辑在 C++、表现在蓝图」边界。

## 2. 模块总览

| 模块 | 文件 | 职责 |
| --- | --- | --- |
| 角色基类 | `Characters/WarriorCharacter.h/.cpp` | 组合三大组件，作为输入入口与表现层回调的转发中枢 |
| 属性组件 | `Combat/AttributeComponent.h/.cpp` | 生命/魔法/能量三条资源的数值与规则、伤害结算、死亡判定 |
| 战斗组件 | `Combat/CombatComponent.h/.cpp` | 攻击判定窗口、胶囊扫掠命中检测、伤害传递 |
| 连招组件 | `Combat/ComboComponent.h/.cpp` | 连击计数、攻击/闪避/爆气(EX) 状态机、爆气计时与移速切换 |
| 动画通知 | `Combat/WarriorAnimNotifies.h/.cpp` | `Attack Window` / `Combo Window` 两个 `AnimNotifyState`，在判定帧驱动 C++ |
| 类型定义 | `Combat/BlueBattleTypes.h` | `EAttackType` 枚举、`FWarriorAttributes` / `FComboMoveData`（可配 DataTable） |
| 对局模式 | `Core/BlueBattleGameMode.h/.cpp` | 监听死亡、判定胜负、广播结算事件 |

## 3. 设计原则

1. **单一数据源**：数值与状态只存在于 C++ 组件中，表现层通过事件被动接收，不持有权威数据。
2. **逻辑/表现分离**：所有「决定发生什么」的代码在 C++；所有「看起来/听起来如何」的代码在蓝图。
   - C++ → 蓝图：`BlueprintImplementableEvent`（如 `OnPlayAttackMontage`、`OnDeathFeedback`）与 `BlueprintAssignable` 委托。
   - 蓝图 → C++：`BlueprintCallable` 入口（如 `LightAttack` / `ActivateBurst` / `ApplyDamage`）。
3. **组件化 / 高内聚**：属性、命中、连招各自独立成 `UActorComponent`，可单独复用与测试，角色类只做组合与转发。
4. **数据驱动可扩展**：`FWarriorAttributes` / `FComboMoveData` 为 `USTRUCT(BlueprintType)` 且继承 `FTableRowBase`，可用 DataTable/DataAsset 配置数值与连招。

## 4. 关键数据流

### 4.1 攻击命中链
```
输入(轻击/重击) → WarriorCharacter::LightAttack/HeavyAttack
  → ComboComponent::PerformAttack           // 状态机：进攻/清闪避、计数、区分普通/EX
    → OnPlayAttackMontage (蓝图实现)          // 表现层：按段选蒙太奇 + 音效
蒙太奇判定帧 → AnimNotifyState_AttackWindow
  → CombatComponent::BeginAttackWindow/EndAttackWindow
    → PerformHitDetection (胶囊扫掠)          // 命中检测，单次挥击去重
      → AttributeComponent::ApplyDamage       // 目标扣血 + 死亡判定
        → OnHitTarget / OnHealthChanged (委托) // 表现层：受击特效、血条刷新
```

### 4.2 属性与死亡
```
AttributeComponent::TickComponent   // 魔法回复、能量衰减、bCanBurst 刷新、Health<=0 判死
  → OnHealthChanged/OnManaChanged/OnEnergyChanged (委托) → UI 血条/蓝条/能量条
  → OnDeath (委托) → WarriorCharacter::HandleDeath → OnDeathFeedback (蓝图) + GameMode 判负
```

### 4.3 爆气 / EX
```
输入(爆气) → WarriorCharacter::Burst → ComboComponent::ActivateBurst
  → 校验 AttributeComponent::CanBurst() → ConsumeEnergy(消耗能量)
  → 提升 MaxWalkSpeed、置 bExplosion、开启计时器(BurstDuration 后 EndBurst 恢复)
  → OnBurstStateChanged (委托) → 表现层爆气特效
```

## 5. C++ / 蓝图边界（对外接口）

- **输入入口（蓝图/PlayerController 调用）**：`LightAttack` / `HeavyAttack` / `Dodge` / `Burst`。
- **表现层回调（蓝图实现）**：`OnAttackPerformed` / `OnDodgePerformed` / `OnHitReaction` / `OnDeathFeedback`；组件级 `OnPlayAttackMontage` / `OnPlayDodgeMontage`。
- **状态查询（AnimBP 读取）**：`IsAttacking` / `GetComboStep` / `IsAlive`。
- **事件订阅（UI/特效）**：`OnHealthChanged` / `OnManaChanged` / `OnEnergyChanged` / `OnDeath` / `OnHitTarget` / `OnComboStepChanged` / `OnBurstStateChanged` / `OnMatchEnded`。

## 6. 迁移说明与取舍

- 原项目为**纯蓝图实现且已可玩**。本次将核心逻辑重写为上述 C++ 组件，`BP_Warrior` 与其 `AttributeSystem` / `ComboSystem` 组件已 **Reparent** 到对应 C++ 基类，使 C++ 成为可编译、可复用的战斗框架。
- 为**保证既有可玩性与 UI/数值平衡不被破坏**，蓝图侧的表现与数据绑定保持现状；C++ 组件字段采用独立、语义清晰的命名（`Health/Mana/Energy/bCanBurst/ComboCount`），避免与蓝图变量产生隐式耦合。
- 若后续要让运行时完全由 C++ 驱动，只需将蓝图中的连招/伤害节点替换为对上述 C++ 入口的调用，并把蒙太奇选择移入 `OnPlayAttackMontage`——接口已按此预留。

## 7. 面试讲解要点

- 用 `UActorComponent` 做**组件化战斗系统**，解耦属性/命中/连招，符合 UE 的组合优于继承实践。
- 用 **委托 + BlueprintImplementableEvent** 建立稳定的「逻辑/表现」契约，美术与程序可并行迭代。
- 用 **AnimNotifyState** 把攻击判定帧与代码解耦，帧数据由动画师在蒙太奇上可视化调节。
- 命中检测做了**单次挥击去重**（`HitActorsThisWindow`），避免一次攻击多帧重复扣血。
- 数值/连招**数据驱动**（`FTableRowBase`），便于策划用表格调参。
