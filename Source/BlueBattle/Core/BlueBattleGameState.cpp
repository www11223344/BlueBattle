// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BlueBattleGameState.h"

#include "UI/ViewModels/BlueBattleMatchResultViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleGameState)

UBlueBattleMatchResultViewModel* ABlueBattleGameState::GetMatchResultViewModel()
{
	if (!MatchResultViewModel)
	{
		MatchResultViewModel = NewObject<UBlueBattleMatchResultViewModel>(this);
	}
	return MatchResultViewModel;
}
