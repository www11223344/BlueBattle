// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameStateComponent.h"

#include "BlueBattleExperienceManagerComponent.generated.h"

namespace UE::GameFeatures { struct FResult; }

class UBlueBattleExperienceDefinition;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBlueBattleExperienceLoaded, const UBlueBattleExperienceDefinition* /*Experience*/);

enum class EBlueBattleExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	ExecutingActions,
	Loaded,
	Deactivating
};

/**
 * 挂在 GameState 上，负责加载当前 Experience：异步加载资源、激活 GameFeature 插件、执行 Actions，
 * 完成后按优先级广播 OnExperienceLoaded（对应 Lyra 的 ULyraExperienceManagerComponent）。
 */
UCLASS()
class BLUEBATTLE_API UBlueBattleExperienceManagerComponent final : public UGameStateComponent
{
	GENERATED_BODY()

public:
	UBlueBattleExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SetCurrentExperience(FPrimaryAssetId ExperienceId);

	/** 已加载则立即调用，否则在加载完成时先于其他回调调用 */
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnBlueBattleExperienceLoaded::FDelegate&& Delegate);
	void CallOrRegister_OnExperienceLoaded(FOnBlueBattleExperienceLoaded::FDelegate&& Delegate);
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnBlueBattleExperienceLoaded::FDelegate&& Delegate);

	const UBlueBattleExperienceDefinition* GetCurrentExperienceChecked() const;
	bool IsExperienceLoaded() const;

private:
	UFUNCTION()
	void OnRep_CurrentExperience();

	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnExperienceFullLoadCompleted();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperience)
	TObjectPtr<const UBlueBattleExperienceDefinition> CurrentExperience;

	EBlueBattleExperienceLoadState LoadState = EBlueBattleExperienceLoadState::Unloaded;

	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	FOnBlueBattleExperienceLoaded OnExperienceLoaded_HighPriority;
	FOnBlueBattleExperienceLoaded OnExperienceLoaded;
	FOnBlueBattleExperienceLoaded OnExperienceLoaded_LowPriority;
};
