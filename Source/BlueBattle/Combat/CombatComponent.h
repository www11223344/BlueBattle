// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

/** 命中目标时广播：被击中的 Actor / 实际造成的伤害 / 命中结果。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHitTarget, AActor*, HitActor, float, DamageDealt, const FHitResult&, HitResult);

/**
 * 战斗组件：负责攻击窗口开关、射线/扫掠命中检测与伤害传递。
 * 由 AnimNotifyState 在攻击动画的判定帧调用 BeginAttackWindow / EndAttackWindow。
 */
UCLASS(ClassGroup = (BlueBattle), meta = (BlueprintSpawnableComponent))
class BLUEBATTLE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 命中事件，供表现层（特效/音效/顿帧）订阅 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnHitTarget OnHitTarget;

	/** 开启攻击判定窗口。DamageMultiplier 来自当前连招段。 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginAttackWindow(float DamageMultiplier = 1.f);

	/** 关闭攻击判定窗口。 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndAttackWindow();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttackWindowOpen() const { return bAttackWindowOpen; }

protected:
	virtual void BeginPlay() override;

	/** 在攻击窗口内执行一次命中检测（从角色前方做胶囊扫掠） */
	void PerformHitDetection();

	/** 攻击判定的检测半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TraceRadius = 60.f;

	/** 攻击判定距离（角色前方） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float TraceRange = 120.f;

	/** 检测使用的 Object 通道（默认 Pawn） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	/** 是否绘制调试图形 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Debug")
	bool bDrawDebug = false;

private:
	/** 攻击窗口是否开启 */
	bool bAttackWindowOpen = false;

	/** 当前段的伤害倍率 */
	float CurrentDamageMultiplier = 1.f;

	/** 本次攻击窗口内已命中的 Actor，避免单次挥击重复扣血 */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActorsThisWindow;
};
