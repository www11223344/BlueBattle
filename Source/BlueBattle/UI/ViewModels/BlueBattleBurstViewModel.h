// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MVVMViewModelBase.h"

#include "BlueBattleBurstViewModel.generated.h"

class APawn;
class UAttributeComponent;
class UComboComponent;
class UCommonLocalPlayer;
class ULocalPlayer;

/**
 * 爆气状态 ViewModel（MVVM）：跟随本地玩家当前控制的 Pawn，汇总属性组件（能量是否达到 EX 阈值、是否死亡）
 * 与连招组件（是否爆气中），给出「现在能否爆气」及对应的显示文本 / 颜色，供 HUD 的爆气提示控件绑定。
 * 判定条件与 UBlueBattleUnitSubsystem::TryActivateBurst 一致：存活、能量达到 EX 阈值、不在爆气中。
 */
UCLASS(BlueprintType, DisplayName = "BlueBattle Burst Viewmodel")
class BLUEBATTLE_API UBlueBattleBurstViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Burst")
	void InitializeForLocalPlayer(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintCallable, Category = "Burst")
	void SetFromActor(AActor* Actor);

	void Deinitialize();

	bool CanBurst() const { return bCanBurst; }
	bool IsBursting() const { return bIsBursting; }

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Burst")
	FText GetStatusText() const;

	UFUNCTION(BlueprintPure, FieldNotify, Category = "Burst")
	FLinearColor GetStatusColor() const;

protected:
	/** 现在按下爆气能否成功 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "CanBurst", Category = "Burst")
	bool bCanBurst = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "IsBursting", Category = "Burst")
	bool bIsBursting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Burst")
	FLinearColor ReadyColor = FLinearColor(0.1f, 0.75f, 0.25f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Burst")
	FLinearColor BurstingColor = FLinearColor(1.f, 0.45f, 0.05f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Burst")
	FLinearColor UnavailableColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.6f);

private:
	void HandlePawnSet(UCommonLocalPlayer* LocalPlayer, APawn* Pawn);

	UFUNCTION()
	void HandleEnergyChanged(float NewEnergy, float NewMaxEnergy, float Percent);

	UFUNCTION()
	void HandleDeath(AActor* Killer);

	UFUNCTION()
	void HandleBurstStateChanged(bool bActive);

	void BindComponents(UAttributeComponent* InAttributeComponent, UComboComponent* InComboComponent);
	void Refresh();

	TWeakObjectPtr<UAttributeComponent> AttributeComponent;
	TWeakObjectPtr<UComboComponent> ComboComponent;
	TWeakObjectPtr<UCommonLocalPlayer> TrackedLocalPlayer;
	FDelegateHandle PawnSetHandle;
};
