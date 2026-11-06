// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BlueBattlePawnExtensionComponent.h"

#include "BlueBattleGameplayTags.h"
#include "BlueBattleLogChannels.h"
#include "Characters/BlueBattlePawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattlePawnExtensionComponent)

const FName UBlueBattlePawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

UBlueBattlePawnExtensionComponent::UBlueBattlePawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UBlueBattlePawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PawnData);
}

void UBlueBattlePawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	ensureAlwaysMsgf(GetPawn<APawn>() != nullptr, TEXT("BlueBattlePawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	RegisterInitStateFeature();
}

void UBlueBattlePawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	ensure(TryToChangeInitState(BlueBattleGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UBlueBattlePawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UBlueBattlePawnExtensionComponent::SetPawnData(const UBlueBattlePawnData* InPawnData)
{
	check(InPawnData);

	APawn* Pawn = GetPawnChecked<APawn>();
	if (Pawn->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogBlueBattle, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;
	Pawn->ForceNetUpdate();

	CheckDefaultInitialization();
}

void UBlueBattlePawnExtensionComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UBlueBattlePawnExtensionComponent::HandleControllerChanged()
{
	CheckDefaultInitialization();
}

void UBlueBattlePawnExtensionComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UBlueBattlePawnExtensionComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UBlueBattlePawnExtensionComponent::CheckDefaultInitialization()
{
	// 先推进依赖本组件的其他 Feature
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = {
		BlueBattleGameplayTags::InitState_Spawned,
		BlueBattleGameplayTags::InitState_DataAvailable,
		BlueBattleGameplayTags::InitState_DataInitialized,
		BlueBattleGameplayTags::InitState_GameplayReady };

	ContinueInitStateChain(StateChain);
}

bool UBlueBattlePawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	if (!CurrentState.IsValid() && DesiredState == BlueBattleGameplayTags::InitState_Spawned)
	{
		return Pawn != nullptr;
	}

	if (CurrentState == BlueBattleGameplayTags::InitState_Spawned && DesiredState == BlueBattleGameplayTags::InitState_DataAvailable)
	{
		if (!PawnData)
		{
			return false;
		}

		if ((Pawn->HasAuthority() || Pawn->IsLocallyControlled()) && !GetController<AController>())
		{
			return false;
		}

		return true;
	}

	if (CurrentState == BlueBattleGameplayTags::InitState_DataAvailable && DesiredState == BlueBattleGameplayTags::InitState_DataInitialized)
	{
		return Manager->HaveAllFeaturesReachedInitState(Pawn, BlueBattleGameplayTags::InitState_DataAvailable);
	}

	if (CurrentState == BlueBattleGameplayTags::InitState_DataInitialized && DesiredState == BlueBattleGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UBlueBattlePawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName != NAME_ActorFeatureName && Params.FeatureState == BlueBattleGameplayTags::InitState_DataAvailable)
	{
		CheckDefaultInitialization();
	}
}
