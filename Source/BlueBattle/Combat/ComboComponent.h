// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/BlueBattleTypes.h"
#include "ComboComponent.generated.h"

/** 连招段数变化时广播：当前 ComboIndex / 攻击类型。供 AnimBP / UI 使用。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboStepChanged, int32, ComboIndex, EAttackType, AttackType);

/** 连招重置（回到待机）时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboReset);

/** 爆气状态变化时广播：是否进入爆气。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBurstStateChanged, bool, bActive);

/**
 * 连招系统组件：管理连击计数、攻击/闪避/爆气(EX) 状态机。
 *
 * 迁移自蓝图 ComboSystem：
 *   - 轻击/重击：普通段 & 爆气中的 EX 段，按 ComboIndex 循环取蒙太奇
 *   - 闪避：清除攻击状态、进入闪避状态
 *   - 爆气(EX)：消耗能量、提升移动速度、持续 N 秒后自动结束
 *
 * 逻辑（状态、计数、能量消耗、爆气计时、移动速度）全部在 C++；
 * 表现层（选择并播放蒙太奇、音效、闪避方向）通过 BlueprintImplementableEvent 交回蓝图。
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

	/** 轻击（对应蓝图 轻击_system）。爆气中自动走 EX 轻击。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void PerformLightAttack();

	/** 重击（对应蓝图 重击_system）。爆气中自动走 EX 重击。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void PerformHeavyAttack();

	/** 闪避（对应蓝图 闪避）。清攻击状态、进入闪避。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void PerformDodge();

	/** 触发爆气/EX。需要能量足够（AttributeComponent::CanBurst()）。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ActivateBurst();

	/** 攻击动画结束时调用（AnimNotify / 蒙太奇结束），清除攻击状态。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void EndAttack();

	/** 强制重置连招计数回到待机。 */
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void ResetCombo();

	// ---- 供 AnimNotify 使用的连招窗口（可选：更精细的续招手感）----

	/** 打开连招窗口（蒙太奇上的 AnimNotify 调用）。 */
	UFUNCTION(BlueprintCallable, Category = "Combo|Window")
	void OpenComboWindow() { bComboWindowOpen = true; }

	/** 关闭连招窗口。 */
	UFUNCTION(BlueprintCallable, Category = "Combo|Window")
	void CloseComboWindow() { bComboWindowOpen = false; }

	// ---- 查询 ----

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

	// ---- 表现层回调（蓝图实现：选择并播放蒙太奇 / 音效 / 闪避方向）----

	/**
	 * 请求播放一段攻击。蓝图根据 AttackType + bEX 选择对应蒙太奇数组，
	 * 播放第 (ComboIndex % 数组长度) 段，并处理音效 / 当前技能记录。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combo|Feedback")
	void OnPlayAttackMontage(EAttackType AttackType, bool bEX, int32 ComboStep);

	/** 请求播放闪避表现（蓝图按角色朝向选择闪避蒙太奇）。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combo|Feedback")
	void OnPlayDodgeMontage();

protected:
	virtual void BeginPlay() override;

	/** 执行一段攻击的通用流程 */
	void PerformAttack(EAttackType AttackType);

	/** 爆气结束（计时器回调） */
	void EndBurst();

	/** 取属主上的属性组件（用于能量判定/消耗） */
	class UAttributeComponent* GetAttribute() const;

	// ---- 配置 ----

	/** 爆气持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Burst")
	float BurstDuration = 15.f;

	/** 爆气期间的移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Burst")
	float BurstMoveSpeed = 600.f;

	/** 常态移动速度（爆气结束后恢复） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Burst")
	float NormalMoveSpeed = 400.f;

	/** 爆气消耗的能量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo|Burst")
	float BurstEnergyCost = 50.f;

	// ---- 运行时状态 ----

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
	bool bComboWindowOpen = false;

private:
	/** 爆气计时器句柄 */
	FTimerHandle BurstTimerHandle;
};
