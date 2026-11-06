// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"

#include "BlueBattlePawnExtensionComponent.generated.h"

class UBlueBattlePawnData;

/**
 * 协调 Pawn 上各组件的初始化顺序（InitState.Spawned → DataAvailable → DataInitialized → GameplayReady），
 * 并持有生成时由 GameMode 写入的 PawnData（对应 Lyra 的 ULyraPawnExtensionComponent，去掉了 GAS 部分）。
 */
UCLASS()
class BLUEBATTLE_API UBlueBattlePawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	UBlueBattlePawnExtensionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const FName NAME_ActorFeatureName;

	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	UFUNCTION(BlueprintPure, Category = "BlueBattle|Pawn")
	static UBlueBattlePawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<UBlueBattlePawnExtensionComponent>() : nullptr; }

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UBlueBattlePawnData* InPawnData);

	/** 由 Pawn 在控制器变化时调用 */
	void HandleControllerChanged();

	/** 由 Pawn 在 PlayerState 复制到达时调用 */
	void HandlePlayerStateReplicated();

	/** 由 Pawn 在输入组件创建完成时调用 */
	void SetupPlayerInputComponent();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_PawnData();

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "BlueBattle|Pawn")
	TObjectPtr<const UBlueBattlePawnData> PawnData;
};
