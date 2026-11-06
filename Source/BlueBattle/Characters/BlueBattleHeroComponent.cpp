// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/BlueBattleHeroComponent.h"

#include "BlueBattleGameplayTags.h"
#include "BlueBattleLogChannels.h"
#include "Characters/BlueBattlePawnData.h"
#include "Characters/BlueBattlePawnExtensionComponent.h"
#include "Characters/WarriorCharacter.h"
#include "Components/GameFrameworkComponentManager.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Input/BlueBattleInputComponent.h"
#include "InputActionValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleHeroComponent)

const FName UBlueBattleHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UBlueBattleHeroComponent::NAME_ActorFeatureName("Hero");

UBlueBattleHeroComponent::UBlueBattleHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBlueBattleHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogBlueBattle, Error, TEXT("BlueBattleHeroComponent on [%s] must be placed on a Pawn."), *GetNameSafe(GetOwner()));
		return;
	}

	RegisterInitStateFeature();
}

void UBlueBattleHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	BindOnActorInitStateChanged(UBlueBattlePawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	ensure(TryToChangeInitState(BlueBattleGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UBlueBattleHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool UBlueBattleHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == BlueBattleGameplayTags::InitState_Spawned)
	{
		return Pawn != nullptr;
	}

	if (CurrentState == BlueBattleGameplayTags::InitState_Spawned && DesiredState == BlueBattleGameplayTags::InitState_DataAvailable)
	{
		if (!GetPlayerState<APlayerState>())
		{
			return false;
		}

		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			const AController* Controller = GetController<AController>();
			const bool bHasControllerPairedWithPS = Controller && Controller->PlayerState && Controller->PlayerState->GetOwner() == Controller;
			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		if (Pawn->IsLocallyControlled() && !Pawn->IsBotControlled())
		{
			const APlayerController* PC = GetController<APlayerController>();
			if (!Pawn->InputComponent || !PC || !PC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}

	if (CurrentState == BlueBattleGameplayTags::InitState_DataAvailable && DesiredState == BlueBattleGameplayTags::InitState_DataInitialized)
	{
		return GetPlayerState<APlayerState>() && Manager->HasFeatureReachedInitState(Pawn, UBlueBattlePawnExtensionComponent::NAME_ActorFeatureName, BlueBattleGameplayTags::InitState_DataInitialized);
	}

	if (CurrentState == BlueBattleGameplayTags::InitState_DataInitialized && DesiredState == BlueBattleGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UBlueBattleHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == BlueBattleGameplayTags::InitState_DataAvailable && DesiredState == BlueBattleGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		if (GetController<APlayerController>() && Pawn && Pawn->InputComponent)
		{
			InitializePlayerInput(Pawn->InputComponent);
		}
	}
}

void UBlueBattleHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UBlueBattlePawnExtensionComponent::NAME_ActorFeatureName && Params.FeatureState == BlueBattleGameplayTags::InitState_DataInitialized)
	{
		CheckDefaultInitialization();
	}
}

void UBlueBattleHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		BlueBattleGameplayTags::InitState_Spawned,
		BlueBattleGameplayTags::InitState_DataAvailable,
		BlueBattleGameplayTags::InitState_DataInitialized,
		BlueBattleGameplayTags::InitState_GameplayReady };

	ContinueInitStateChain(StateChain);
}

void UBlueBattleHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	APawn* Pawn = GetPawn<APawn>();
	APlayerController* PC = GetController<APlayerController>();
	const ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr;
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (!Pawn || !Subsystem)
	{
		return;
	}

	Subsystem->ClearAllMappings();

	const UBlueBattlePawnExtensionComponent* PawnExtComp = UBlueBattlePawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	const UBlueBattlePawnData* PawnData = PawnExtComp ? PawnExtComp->GetPawnData<UBlueBattlePawnData>() : nullptr;
	if (const UBlueBattleInputConfig* InputConfig = PawnData ? PawnData->InputConfig.Get() : nullptr)
	{
		UBlueBattleInputComponent* BlueBattleIC = Cast<UBlueBattleInputComponent>(PlayerInputComponent);
		if (ensureMsgf(BlueBattleIC, TEXT("Input component of [%s] is not a UBlueBattleInputComponent, check DefaultInputComponentClass."), *GetNameSafe(Pawn)))
		{
			TArray<uint32> BindHandles;
			BlueBattleIC->BindActionInputs(InputConfig, this, &ThisClass::Input_ActionPressed, BindHandles);

			BlueBattleIC->BindNativeAction(InputConfig, BlueBattleGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false);
			BlueBattleIC->BindNativeAction(InputConfig, BlueBattleGameplayTags::InputTag_Look, ETriggerEvent::Triggered, this, &ThisClass::Input_Look, false);
			BlueBattleIC->BindNativeAction(InputConfig, BlueBattleGameplayTags::InputTag_ConsumeMP, ETriggerEvent::Triggered, this, &ThisClass::Input_ConsumeMP, false);
		}
	}

	ensure(!bReadyToBindInputs);
	bReadyToBindInputs = true;

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(PC, NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(Pawn, NAME_BindInputsNow);
}

void UBlueBattleHeroComponent::Input_ActionPressed(FGameplayTag InputTag)
{
	if (AWarriorCharacter* Warrior = GetPawn<AWarriorCharacter>())
	{
		Warrior->HandleInputAction(InputTag);
	}
}

void UBlueBattleHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn || !Pawn->GetController())
	{
		return;
	}

	// X = 右，Y = 前；沿角色自身朝向移动
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	if (Value.Y != 0.f)
	{
		Pawn->AddMovementInput(Pawn->GetActorForwardVector(), Value.Y);
	}
	if (Value.X != 0.f)
	{
		Pawn->AddMovementInput(Pawn->GetActorRightVector(), Value.X);
	}

	if (AWarriorCharacter* Warrior = Cast<AWarriorCharacter>(Pawn))
	{
		Warrior->UpdateDodgeDirection(Value);
	}
}

void UBlueBattleHeroComponent::Input_Look(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();
	if (Value.X != 0.f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}
	if (Value.Y != 0.f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UBlueBattleHeroComponent::Input_ConsumeMP(const FInputActionValue& InputActionValue)
{
	if (AWarriorCharacter* Warrior = GetPawn<AWarriorCharacter>())
	{
		Warrior->HandleInputAction(BlueBattleGameplayTags::InputTag_ConsumeMP);
	}
}
