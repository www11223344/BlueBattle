// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/BlueBattleExperienceManagerComponent.h"

#include "BlueBattleLogChannels.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameModes/BlueBattleExperienceActionSet.h"
#include "GameModes/BlueBattleExperienceDefinition.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleExperienceManagerComponent)

UBlueBattleExperienceManagerComponent::UBlueBattleExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UBlueBattleExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
}

void UBlueBattleExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ExperienceId);
	const UBlueBattleExperienceDefinition* Experience = Cast<UBlueBattleExperienceDefinition>(AssetPath.TryLoad());
	if (!ensureMsgf(Experience, TEXT("Failed to load experience %s"), *ExperienceId.ToString()))
	{
		return;
	}

	check(CurrentExperience == nullptr);
	CurrentExperience = Experience;
	StartExperienceLoad();
}

void UBlueBattleExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(FOnBlueBattleExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

void UBlueBattleExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FOnBlueBattleExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

void UBlueBattleExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(FOnBlueBattleExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
}

const UBlueBattleExperienceDefinition* UBlueBattleExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	check(LoadState == EBlueBattleExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

bool UBlueBattleExperienceManagerComponent::IsExperienceLoaded() const
{
	return LoadState == EBlueBattleExperienceLoadState::Loaded && CurrentExperience != nullptr;
}

void UBlueBattleExperienceManagerComponent::OnRep_CurrentExperience()
{
	StartExperienceLoad();
}

void UBlueBattleExperienceManagerComponent::StartExperienceLoad()
{
	check(CurrentExperience != nullptr);
	check(LoadState == EBlueBattleExperienceLoadState::Unloaded);

	UE_LOG(LogBlueBattleExperience, Log, TEXT("EXPERIENCE: StartExperienceLoad(%s)"), *CurrentExperience->GetPrimaryAssetId().ToString());

	LoadState = EBlueBattleExperienceLoadState::Loading;

	UAssetManager& AssetManager = UAssetManager::Get();

	TArray<FPrimaryAssetId> BundleAssetList;
	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UBlueBattleExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	TArray<FName> BundlesToLoad;
	if (GIsEditor || OwnerNetMode != NM_DedicatedServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (GIsEditor || OwnerNetMode != NM_Client)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	const TSharedPtr<FStreamableHandle> Handle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList, BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);

	const FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnExperienceLoadComplete);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);
		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
		{
			OnAssetsLoadedDelegate.ExecuteIfBound();
		}));
	}
}

void UBlueBattleExperienceManagerComponent::OnExperienceLoadComplete()
{
	check(LoadState == EBlueBattleExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	UE_LOG(LogBlueBattleExperience, Log, TEXT("EXPERIENCE: OnExperienceLoadComplete(%s)"), *CurrentExperience->GetPrimaryAssetId().ToString());

	GameFeaturePluginURLs.Reset();

	auto CollectGameFeaturePluginURLs = [this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, PluginURL))
			{
				GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("Failed to find plugin URL from PluginName %s for %s"), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}
	};

	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);
	for (const TObjectPtr<UBlueBattleExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet)
		{
			CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
		}
	}

	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = EBlueBattleExperienceLoadState::LoadingGameFeatures;
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete));
		}
	}
	else
	{
		OnExperienceFullLoadCompleted();
	}
}

void UBlueBattleExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	--NumGameFeaturePluginsLoading;
	if (NumGameFeaturePluginsLoading == 0)
	{
		OnExperienceFullLoadCompleted();
	}
}

void UBlueBattleExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
	check(LoadState != EBlueBattleExperienceLoadState::Loaded);

	LoadState = EBlueBattleExperienceLoadState::ExecutingActions;

	FGameFeatureActivatingContext Context;
	if (const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld()))
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	auto ActivateListOfActions = [&Context](const TArray<TObjectPtr<UGameFeatureAction>>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action)
			{
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	ActivateListOfActions(CurrentExperience->Actions);
	for (const TObjectPtr<UBlueBattleExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet)
		{
			ActivateListOfActions(ActionSet->Actions);
		}
	}

	LoadState = EBlueBattleExperienceLoadState::Loaded;

	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();
}

void UBlueBattleExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
	}

	if (LoadState != EBlueBattleExperienceLoadState::Loaded)
	{
		return;
	}

	LoadState = EBlueBattleExperienceLoadState::Deactivating;

	FGameFeatureDeactivatingContext Context(TEXT(""), [](FStringView) {});
	if (const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld()))
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	auto DeactivateListOfActions = [&Context](const TArray<TObjectPtr<UGameFeatureAction>>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			if (Action)
			{
				Action->OnGameFeatureDeactivating(Context);
				Action->OnGameFeatureUnregistering();
			}
		}
	};

	DeactivateListOfActions(CurrentExperience->Actions);
	for (const TObjectPtr<UBlueBattleExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet)
		{
			DeactivateListOfActions(ActionSet->Actions);
		}
	}

	LoadState = EBlueBattleExperienceLoadState::Unloaded;
	CurrentExperience = nullptr;
}
