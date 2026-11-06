// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BlueBattleGameMode.h"
#include "Core/BlueBattleGameState.h"
#include "Player/BlueBattlePlayerController.h"
#include "Player/BlueBattlePlayerState.h"
#include "UI/BlueBattleHUD.h"

ABlueBattleGameMode::ABlueBattleGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Pawn 仍由蓝图 GameMode 指定；蓝图子类中显式设置过的类不受这里影响。
	GameStateClass = ABlueBattleGameState::StaticClass();
	PlayerControllerClass = ABlueBattlePlayerController::StaticClass();
	PlayerStateClass = ABlueBattlePlayerState::StaticClass();
	HUDClass = ABlueBattleHUD::StaticClass();
}

void ABlueBattleGameMode::NotifyWarriorDied(int32 DeadPlayerIndex)
{
	if (bMatchOver)
	{
		return;
	}

	bMatchOver = true;

	// 双人对战：一方死亡则另一方获胜（0 <-> 1）
	const int32 Winner = (DeadPlayerIndex == 0) ? 1 : 0;
	OnMatchEnded.Broadcast(Winner);
}
