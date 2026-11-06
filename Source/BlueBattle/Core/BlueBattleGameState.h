// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModularGameState.h"

#include "BlueBattleGameState.generated.h"

class UBlueBattleMatchResultViewModel;

UCLASS(Config = Game)
class BLUEBATTLE_API ABlueBattleGameState : public AModularGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Match")
	UBlueBattleMatchResultViewModel* GetMatchResultViewModel();

private:
	UPROPERTY(Transient)
	TObjectPtr<UBlueBattleMatchResultViewModel> MatchResultViewModel;
};
