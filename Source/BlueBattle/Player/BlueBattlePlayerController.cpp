// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BlueBattlePlayerController.h"

#include "Core/BlueBattleGameMode.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattlePlayerController)

ABlueBattlePlayerController::ABlueBattlePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABlueBattlePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	TryInitializeLocalPlayerUI();
}

void ABlueBattlePlayerController::BeginPlay()
{
	Super::BeginPlay();
	TryInitializeLocalPlayerUI();
}

void ABlueBattlePlayerController::TryInitializeLocalPlayerUI()
{
	const bool bBegunPlay = HasActorBegunPlay() || IsActorBeginningPlay();
	if (bLocalPlayerUIInitialized || !bBegunPlay || !GetLocalPlayer())
	{
		return;
	}

	if (ABlueBattleGameMode* GameMode = GetWorld()->GetAuthGameMode<ABlueBattleGameMode>())
	{
		bLocalPlayerUIInitialized = true;
		GameMode->InitializeLocalPlayerUI(this);
	}
}
