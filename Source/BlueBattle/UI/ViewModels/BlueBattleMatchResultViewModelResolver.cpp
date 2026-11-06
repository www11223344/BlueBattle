// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModels/BlueBattleMatchResultViewModelResolver.h"

#include "Blueprint/UserWidget.h"
#include "Core/BlueBattleGameState.h"
#include "Engine/World.h"
#include "UI/ViewModels/BlueBattleMatchResultViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleMatchResultViewModelResolver)

UObject* UBlueBattleMatchResultViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	const UWorld* World = UserWidget ? UserWidget->GetWorld() : nullptr;
	ABlueBattleGameState* GameState = World ? World->GetGameState<ABlueBattleGameState>() : nullptr;
	return GameState ? GameState->GetMatchResultViewModel() : nullptr;
}

#if WITH_EDITOR
bool UBlueBattleMatchResultViewModelResolver::DoesSupportViewModelClass(const UClass* Class) const
{
	return Class && Class->IsChildOf(UBlueBattleMatchResultViewModel::StaticClass());
}
#endif
