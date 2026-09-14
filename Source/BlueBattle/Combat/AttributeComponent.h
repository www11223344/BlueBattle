// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

/** 生命值变化时广播：新值 / 最大值 / 归一化比例(0-1)。UI 血条订阅此事件即可。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHPChanged, float, NewHP, float, MaxHP, float, HPPercent);

/** 魔法值变化时广播：新值 / 最大值 / 归一化比例(0-1)。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMPChanged, float, NewMP, float, MaxMP, float, MPPercent);

/** 能量值变化时广播：新值 / 最大值 / 归一化比例(0-1)。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnergyValueChanged, float, NewEnergyValue, float, MaxEnergyValue, float, EnergyValuePercent);

/** 死亡时广播。可选传入击杀者。UI/表现层订阅后播放死亡表现。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);

/**
 * 属性系统组件：集中管理角色的三条资源（生命值 / 魔法值 / 能量值），
 * 负责伤害结算、魔法/能量消耗与死亡判定，并通过委托把变化通知给表现层(UI/AnimBP)。
 *
 * 设计原则：数值与规则全部在 C++（单一数据源），表现层只订阅事件、不持有权威数据。
 *   - BeginPlay：HP=MaxHP, MP=MaxMP, EnergyValue=MaxEnergyValue
 *   - Tick：MP 向 MaxMP 匀速回复；EnergyValue 向 0 匀速衰减；刷新 CanEX；HP<=0 判死
 *   - ApplyDamage / Heal / ConsumeMP / ConsumeEnergyValue / AddEnergyValue 为对外的原子操作，
 *     每次修改后广播对应委托。
 */
UCLASS(ClassGroup = (BlueBattle), meta = (BlueprintSpawnableComponent))
class BLUEBATTLE_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ---- 事件（供 UI/表现层订阅）----

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnMPChanged OnMPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnEnergyValueChanged OnEnergyValueChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes|Events")
	FOnDeath OnDeath;

	// ---- 运行时资源（C++ 为权威数据源）----

	/** 最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double MaxHP = 100.f;

	/** 当前生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double HP = 100.f;

	/** 最大魔法值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double MaxMP = 100.f;

	/** 当前魔法值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double MP = 100.f;

	/** 最大能量值（爆气/EX 槽） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double MaxEnergyValue = 100.f;

	/** 当前能量值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double EnergyValue = 100.f;

	/** 是否可释放爆气/EX（EnergyValue >= MaxEnergyValue/2），每帧刷新 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	bool CanEX = false;

	/** 基础攻击力（供命中伤害使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double AttackPower = 5.f;

	/** 防御力（伤害减免，默认 0 = 无减免） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	double Defense = 0.f;

	/** 是否已死亡（避免重复触发死亡）*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
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

protected:
	virtual void BeginPlay() override;

	/** 广播当前三条资源给 UI（初始化/变化时调用） */
	void BroadcastAll();

	/** 魔法值每秒回复速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes|Regen")
	float MPRegenSpeed = 10.f;

	/** 能量值每秒衰减速度（回落到 0） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes|Regen")
	float EnergyValueDecaySpeed = 5.f;
	
};
