// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Mass/EntityHandle.h"
#include "AttributeComponent.generated.h"

struct FBlueBattleAttributeFragment;
struct FBlueBattleAttributeRegenFragment;
class UBlueBattleUnitSubsystem;

/** 生命值变化时广播：新值 / 最大值 / 归一化比例(0-1)。UI 血条订阅此事件即可。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHPChanged, float, NewHP, float, MaxHP, float, HPPercent);

/** 魔法值变化时广播：新值 / 最大值 / 归一化比例(0-1)。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMPChanged, float, NewMP, float, MaxMP, float, MPPercent);

/** 能量值变化时广播：新值 / 最大值 / 归一化比例(0-1)。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnergyValueChanged, float, NewEnergyValue, float, MaxEnergyValue, float, EnergyValuePercent);

/** 死亡时广播。可选传入击杀者。UI/表现层订阅后播放死亡表现。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);

/**
 * 属性表现组件：角色三条资源（生命值 / 魔法值 / 能量值）在表现层的只读镜像。
 *
 * 权威数据在 MassEntity 的 FBlueBattleAttributeFragment 中：
 *   - 实体创建时 UBlueBattleAttributeSyncTrait 用本组件的配置（各项上限、攻防、回复速度）初始化 Fragment
 *   - 回复、衰减、判死由 Mass Processor 计算
 *   - UBlueBattleAttributeToActorTranslator 每帧把 Fragment 同步回本组件（ApplyMassSnapshot），值变化时广播委托，
 *     受击 / 死亡时调用 OnDamagedFeedback / OnDeathFeedback 由蓝图子类播放表现
 *   - ApplyDamage / Heal / ConsumeMP / ConsumeEnergyValue / AddEnergyValue 转发到 UBlueBattleUnitSubsystem 写入 Fragment，
 *     下一次 Translator 同步后组件字段与委托才更新
 */
UCLASS(ClassGroup = (BlueBattle), meta = (BlueprintSpawnableComponent))
class BLUEBATTLE_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	// ---- 事件（供 UI/表现层订阅）----

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnMPChanged OnMPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnEnergyValueChanged OnEnergyValueChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnDeath OnDeath;

	// ---- 配置（实体创建时导出到 Fragment，之后只读镜像）----

	/** 最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	double MaxHP = 100.f;

	/** 最大魔法值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	double MaxMP = 100.f;

	/** 最大能量值（爆气/EX 槽） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	double MaxEnergyValue = 100.f;

	/** 基础攻击力（供命中伤害使用） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	double AttackPower = 5.f;

	/** 防御力（伤害减免，默认 0 = 无减免） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	double Defense = 0.f;

	// ---- 运行时镜像（由 Translator 写入，表现层只读）----

	/** 当前生命值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	double HP = 100.f;

	/** 当前魔法值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	double MP = 100.f;

	/** 当前能量值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	double EnergyValue = 100.f;

	/** 是否可释放爆气/EX（EnergyValue >= MaxEnergyValue/2） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	bool CanEX = false;

	/** 是否已死亡 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	bool IsDead = false;

	// ---- 伤害 / 治疗 ----

	/** 应用伤害，返回实际扣除的血量。自动做防御减免与死亡判定。 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float ApplyDamage(float RawDamage, AActor* DamageCauser);

	/** 治疗 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void Heal(float Amount);

	// ---- 魔法 / 能量 ----

	/** 消耗魔法值：MP = Clamp(MP - Cost, 0, MaxMP)。 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ConsumeMP(float Cost);

	/** 消耗能量值。返回是否消耗成功（能量足够）。 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ConsumeEnergyValue(float Cost);

	/** 增加能量值（如命中敌人时积攒）。 */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void AddEnergyValue(float Amount);

	// ---- 查询 ----

	UFUNCTION(BlueprintPure, Category = "Attributes")
	bool IsAlive() const { return !IsDead && HP > 0.f; }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	bool fIsDead() const { return IsDead; }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetHPPercent() const;

	UFUNCTION(BlueprintPure, Category = "Attributes")
	bool fCanEX() const { return CanEX; }

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetAttackPower() const { return AttackPower; }

	// ---- Mass 数据层对接 ----

	FMassEntityHandle GetMassEntity() const { return MassEntity; }

	/** 实体创建时由 Trait 调用：用本组件的配置与当前值初始化 Fragment。 */
	void ExportToMass(FBlueBattleAttributeFragment& OutAttributes, FBlueBattleAttributeRegenFragment& OutRegen) const;

	void BindMassEntity(FMassEntityHandle InEntity) { MassEntity = InEntity; }

	/** Translator 每帧调用：覆盖镜像字段，值有变化才广播对应委托，死亡只广播一次。 */
	void ApplyMassSnapshot(const FBlueBattleAttributeFragment& Attributes, bool bDead);

protected:
	// ---- 表现层回调（蓝图实现）----

	/** 同步到生命值下降时调用（同一帧内的多次伤害合并为一次）：受击动作 / 音效。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes|Feedback")
	void OnDamagedFeedback(float Damage, AActor* DamageCauser);

	/** 同步到死亡时调用一次（先于 OnDeath 广播）：死亡动画 / 关闭碰撞 / 销毁。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Attributes|Feedback")
	void OnDeathFeedback(AActor* Killer);

	virtual void BeginPlay() override;

	void BroadcastHP();
	void BroadcastMP();
	void BroadcastEnergyValue();

	/** 魔法值每秒回复速度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Regen")
	float MPRegenSpeed = 10.f;

	/** 能量值每秒衰减速度（回落到 0） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Regen")
	float EnergyValueDecaySpeed = 5.f;

private:
	/** 未绑定实体时返回 nullptr 并告警（实体在注册后的下一次 Mass 处理阶段才创建） */
	UBlueBattleUnitSubsystem* GetUnitSubsystemForWrite(const TCHAR* Operation) const;

	FMassEntityHandle MassEntity;

	double LastBroadcastHP = -1.0;
	double LastBroadcastMaxHP = -1.0;
	double LastBroadcastMP = -1.0;
	double LastBroadcastMaxMP = -1.0;
	double LastBroadcastEnergyValue = -1.0;
	double LastBroadcastMaxEnergyValue = -1.0;
	bool bDeathBroadcast = false;
};
