// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"

#include "BlueBattleHeroComponent.generated.h"

struct FInputActionValue;

/**
 * 为玩家控制的 Pawn 设置输入：清空本地玩家的映射、按 PawnData 的 InputConfig 绑定输入，
 * 再发出 NAME_BindInputsNow 让 Experience 的 AddInputContextMapping 添加映射
 * （对应 Lyra 的 ULyraHeroComponent，去掉了 GAS/相机部分）。
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class BLUEBATTLE_API UBlueBattleHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBlueBattleHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "BlueBattle|Hero")
	static UBlueBattleHeroComponent* FindHeroComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UBlueBattleHeroComponent>() : nullptr; }

	bool IsReadyToBindInputs() const { return bReadyToBindInputs; }

	static const FName NAME_BindInputsNow;
	static const FName NAME_ActorFeatureName;

	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	void Input_ActionPressed(FGameplayTag InputTag);
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
	void Input_ConsumeMP(const FInputActionValue& InputActionValue);

	bool bReadyToBindInputs = false;
};
