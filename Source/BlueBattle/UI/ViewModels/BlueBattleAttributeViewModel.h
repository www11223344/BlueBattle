// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MVVMViewModelBase.h"

#include "BlueBattleAttributeViewModel.generated.h"

class APawn;
class UAttributeComponent;
class UCommonLocalPlayer;
class ULocalPlayer;

/**
 * 角色属性 ViewModel（MVVM）：把 UAttributeComponent 的生命/魔法/能量暴露为 FieldNotify 字段，
 * Widget 在 Viewmodels 面板中直接绑定字段，无需在蓝图里手动订阅委托。
 *
 * 两种用法：
 *   - InitializeForLocalPlayer：跟随该本地玩家当前控制的 Pawn，Pawn 切换/重生时自动重新绑定（自己的血条）。
 *   - SetAttributeComponent：直接绑定任意角色的属性组件（如对手的血条）。
 */
UCLASS(BlueprintType, DisplayName = "BlueBattle Attribute Viewmodel")
class BLUEBATTLE_API UBlueBattleAttributeViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void InitializeForLocalPlayer(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeComponent(UAttributeComponent* InAttributeComponent);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetFromActor(AActor* Actor);

	void Deinitialize();

	float GetHP() const { return HP; }
	float GetMaxHP() const { return MaxHP; }
	float GetMP() const { return MP; }
	float GetMaxMP() const { return MaxMP; }
	float GetEnergy() const { return Energy; }
	float GetMaxEnergy() const { return MaxEnergy; }
	bool IsCanEX() const { return bCanEX; }
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Attributes")
	float GetHPPercent() const { return MaxHP > 0.f ? HP / MaxHP : 0.f; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Attributes")
	float GetMPPercent() const { return MaxMP > 0.f ? MP / MaxMP : 0.f; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Attributes")
	float GetEnergyPercent() const { return MaxEnergy > 0.f ? Energy / MaxEnergy : 0.f; }

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Attributes")
	float HP = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Attributes")
	float MaxHP = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Attributes")
	float MP = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Attributes")
	float MaxMP = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Attributes")
	float Energy = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Attributes")
	float MaxEnergy = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "IsCanEX", Category = "Attributes")
	bool bCanEX = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "IsDead", Category = "Attributes")
	bool bIsDead = false;

private:
	void HandlePawnSet(UCommonLocalPlayer* LocalPlayer, APawn* Pawn);

	UFUNCTION()
	void HandleHPChanged(float NewHP, float NewMaxHP, float Percent);

	UFUNCTION()
	void HandleMPChanged(float NewMP, float NewMaxMP, float Percent);

	UFUNCTION()
	void HandleEnergyChanged(float NewEnergy, float NewMaxEnergy, float Percent);

	UFUNCTION()
	void HandleDeath(AActor* Killer);

	void PullFromComponent();
	void SetHPValues(float NewHP, float NewMaxHP);
	void SetMPValues(float NewMP, float NewMaxMP);
	void SetEnergyValues(float NewEnergy, float NewMaxEnergy);

	TWeakObjectPtr<UAttributeComponent> AttributeComponent;
	TWeakObjectPtr<UCommonLocalPlayer> TrackedLocalPlayer;
	FDelegateHandle PawnSetHandle;
};
