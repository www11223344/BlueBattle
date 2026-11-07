// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/BlueBattleTypes.h"
#include "Mass/EntityHandle.h"
#include "ComboComponent.generated.h"

struct FBlueBattleComboFragment;
struct FBlueBattleComboParamsFragment;
struct FBlueBattleSkillSetFragment;
class UAnimMontage;
class UBlueBattleUnitSubsystem;

/** 连招段数变化时广播：当前 ComboIndex / 攻击类型。供 AnimBP / UI 使用。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboStepChanged, int32, ComboIndex, EAttackType, AttackType);

/** 连招重置（回到待机）时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboReset);

/** 爆气状态变化时广播：是否进入爆气。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBurstStateChanged, bool, bActive);

/**
 * 连招表现组件：连招 / 闪避 / 爆气(EX) 状态在表现层的只读镜像。
 *
 * 权威状态在 MassEntity 的 FBlueBattleComboFragment 中：
 *   - 输入入口（PerformLightAttack 等）经 UBlueBattleUnitSubsystem 做出招判定并写入 Fragment
 *   - 招式选择与命中伤害由数据层按招式表计算，表现层只经 ReportHit 上报碰撞
 *   - 判定通过后立即回调 OnPlayAttackMontage / OnPlayDodgeMontage，由蓝图播放蒙太奇、音效
 *   - 爆气倒计时、攻击/闪避互斥由 Mass Processor 计算
 *   - UBlueBattleComboToActorTranslator 每帧同步回本组件，状态变化时广播事件并切换移动速度
 *   - 组件 Tick 仅在爆气期间开启，供蓝图子类做逐帧表现（残影）
 */
UCLASS(ClassGroup = (BlueBattle), meta = (BlueprintSpawnableComponent))
class BLUEBATTLE_API UComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UComboComponent();

	// ---- 事件 ----

	/** 段数变化事件（驱动动画/UI） */
	UPROPERTY(BlueprintAssignable, Category = "Combo|Events")
	FOnComboStepChanged OnComboStepChanged;

	/** 连招结束/重置事件 */
	UPROPERTY(BlueprintAssignable, Category = "Combo|Events")
	FOnComboReset OnComboReset;

	/** 爆气状态变化事件 */
	UPROPERTY(BlueprintAssignable, Category = "Combo|Events")
	FOnBurstStateChanged OnBurstStateChanged;

	// ---- 输入入口（供蓝图输入事件 / PlayerController 调用）----

	/** 轻击。爆气中自动走 EX 轻击。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void PerformLightAttack();

	/** 重击。爆气中自动走 EX 重击。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void PerformHeavyAttack();

	/** 闪避：打断攻击、进入闪避。冷却中或已在闪避时返回 false。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	bool PerformDodge();

	/** 触发爆气/EX。需要能量达到 EX 阈值。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ActivateBurst();

	/** 攻击动画结束时调用（AnimNotify / 蒙太奇结束），清除攻击状态。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void EndAttack();

	/** 闪避动画结束时调用，清除闪避与攻击状态。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void EndDodge();

	/** 强制重置连招计数回到待机。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ResetCombo();

	/**
	 * 攻击判定（武器碰撞等）碰到 Target 时调用，由数据层判断是否有效命中并结算伤害与能量。
	 * @return 目标实际损失的生命，未命中为 0；表现层据此播放受击反馈。
	 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	float ReportHit(AActor* Target);

	// ---- 供 AnimNotify 使用的连招窗口 ----

	UFUNCTION(BlueprintCallable, Category = "Combo|Window")
	void OpenComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Combo|Window")
	void CloseComboWindow();

	// ---- 查询（镜像值，比 Fragment 最多晚一帧）----

	UFUNCTION(BlueprintPure, Category = "Combo")
	int32 GetComboCount() const { return ComboCount; }

	UFUNCTION(BlueprintPure, Category = "Combo")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "Combo")
	bool IsEvading() const { return bEvasion; }

	UFUNCTION(BlueprintPure, Category = "Combo")
	bool IsBursting() const { return bExplosion; }

	UFUNCTION(BlueprintPure, Category = "Combo")
	bool IsComboWindowOpen() const { return bComboWindowOpen; }

	// ---- 表现层回调（蓝图实现：播放蒙太奇 / 音效 / 闪避方向）----

	/** 出招判定通过后立即调用。Skill 为数据层选出的本招蒙太奇，蓝图负责播放速率与音效。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combo|Feedback")
	void OnPlayAttackMontage(EAttackType AttackType, bool bEX, int32 ComboStep, UAnimMontage* Skill);

	/** 闪避判定通过后立即调用（蓝图按方向选择闪避蒙太奇；找不到蒙太奇时应调用 EndDodge）。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combo|Feedback")
	void OnPlayDodgeMontage();

	// ---- Mass 数据层对接 ----

	FMassEntityHandle GetMassEntity() const { return MassEntity; }

	/** 实体创建时由 Trait 调用：导出爆气配置与招式表。 */
	void ExportToMass(FBlueBattleComboParamsFragment& OutParams, FBlueBattleSkillSetFragment& OutSkillSet) const;

	void BindMassEntity(FMassEntityHandle InEntity) { MassEntity = InEntity; }

	/** Translator 每帧调用：覆盖镜像字段，状态变化时广播事件、切换移动速度。 */
	void ApplyMassSnapshot(const FBlueBattleComboFragment& Combo, bool bBursting);

protected:
	void PerformAttack(EAttackType AttackType);

	void ApplyMoveSpeed(bool bBursting) const;

	// ---- 配置 ----

	/** 爆气持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Burst")
	float BurstDuration = 15.f;

	/** 爆气期间的移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Burst")
	float BurstMoveSpeed = 600.f;

	/** 常态移动速度（爆气结束后恢复） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Burst")
	float NormalMoveSpeed = 400.f;

	/** 爆气消耗的能量 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Burst")
	float BurstEnergyCost = 50.f;

	/** 闪避冷却（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Dodge")
	float DodgeCooldown = 0.6f;

	/** 轻击招式序列，按连击段数循环 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Skills")
	TArray<TObjectPtr<UAnimMontage>> LightAttackSkills;

	/** 重击招式序列 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Skills")
	TArray<TObjectPtr<UAnimMontage>> HeavyAttackSkills;

	/** 爆气中的轻击招式序列 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Skills")
	TArray<TObjectPtr<UAnimMontage>> EXLightAttackSkills;

	/** 爆气中的重击招式序列 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Skills")
	TArray<TObjectPtr<UAnimMontage>> EXHeavyAttackSkills;

	/** 每招伤害（未列出的招式伤害为 0） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo|Skills")
	TMap<TObjectPtr<UAnimMontage>, float> SkillDamageTable;

	// ---- 运行时镜像（由 Translator 写入）----

	/** 当前连招计数（循环递增，取蒙太奇时对数组长度取模） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	int32 ComboCount = 0;

	/** 是否处于攻击状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bIsAttacking = false;

	/** 是否处于闪避状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bEvasion = false;

	/** 是否处于爆气(EX)状态 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bExplosion = false;

	/** 连招窗口是否开启 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bComboWindowOpen = false;

private:
	/** 未绑定实体时返回 nullptr 并告警 */
	UBlueBattleUnitSubsystem* GetUnitSubsystemForWrite(const TCHAR* Operation) const;

	FMassEntityHandle MassEntity;
};
