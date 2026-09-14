// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combat/BlueBattleTypes.h"
#include "WarriorCharacter.generated.h"

class UAttributeComponent;
class UCombatComponent;
class UComboComponent;

/**
 * 格斗角色基类：整合属性、战斗、连招三大 C++ 组件。
 * BP_Warrior 应 Reparent 到本类，蓝图仅保留动画/特效/资源引用等表现层。
 */
UCLASS()
class BLUEBATTLE_API AWarriorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWarriorCharacter();

	// ---- 输入入口（供 PlayerController / 蓝图输入事件调用）----

	/** 轻击 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void LightAttack();

	/** 重击 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void HeavyAttack();

	/** 闪避 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void Dodge();

	/** 爆气 / EX */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void Burst();

	// ---- 供 AnimBP 读取的状态 ----

	UFUNCTION(BlueprintPure, Category = "Warrior|State")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|State")
	int32 GetComboStep() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|State")
	bool IsAlive() const;

	// ---- 组件访问 ----

	UFUNCTION(BlueprintPure, Category = "Warrior|Components")
	UAttributeComponent* GetAttributeComponent() const { return AttributeComponent; }

	UFUNCTION(BlueprintPure, Category = "Warrior|Components")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "Warrior|Components")
	UComboComponent* GetComboComponent() const { return ComboComponent; }

	// ---- 表现层回调（蓝图实现：播放蒙太奇/特效/音效）----

	/** 请求播放某一段攻击（蓝图负责根据段数选择并播放蒙太奇） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnAttackPerformed(int32 ComboStep, EAttackType AttackType);

	/** 请求播放闪避表现 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnDodgePerformed();

	/** 受击表现（蓝图播放受击动画/特效） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnHitReaction(float DamageDealt, AActor* DamageCauser);

	/** 死亡表现（蓝图播放死亡蒙太奇/结算） */
	UFUNCTION(BlueprintImplementableEvent, Category = "Warrior|Feedback")
	void OnDeathFeedback(AActor* Killer);

protected:
	virtual void BeginPlay() override;

	/** 属性组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UAttributeComponent> AttributeComponent;

	/** 战斗组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	/** 连招组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Warrior|Components")
	TObjectPtr<UComboComponent> ComboComponent;

	/** 闪避冷却（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Warrior|Combat")
	float DodgeCooldown = 0.6f;

private:
	/** 连招段数变化时的内部回调 → 转发到蓝图 OnAttackPerformed */
	UFUNCTION()
	void HandleComboStepChanged(int32 ComboStep, EAttackType AttackType);

	/** 命中目标时的内部回调 */
	UFUNCTION()
	void HandleHitTarget(AActor* HitActor, float DamageDealt, const FHitResult& HitResult);

	/** 死亡内部回调 → 转发到蓝图 OnDeathFeedback */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	float LastDodgeTime = -100.f;
};
