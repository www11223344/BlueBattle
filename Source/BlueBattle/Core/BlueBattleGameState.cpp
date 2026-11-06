// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BlueBattleGameState.h"

#include "GameModes/BlueBattleExperienceManagerComponent.h"
#include "UI/ViewModels/BlueBattleMatchResultViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleGameState)

ABlueBattleGameState::ABlueBattleGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ExperienceManagerComponent = CreateDefaultSubobject<UBlueBattleExperienceManagerComponent>(TEXT("ExperienceManagerComponent"));
}

UBlueBattleMatchResultViewModel* ABlueBattleGameState::GetMatchResultViewModel()
{
	if (!MatchResultViewModel)
	{
		MatchResultViewModel = NewObject<UBlueBattleMatchResultViewModel>(this);
	}
	return MatchResultViewModel;
}
