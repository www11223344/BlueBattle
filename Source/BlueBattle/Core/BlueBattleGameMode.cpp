// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BlueBattleGameMode.h"

ABlueBattleGameMode::ABlueBattleGameMode()
{
	// 默认使用蓝图配置的 Pawn / Controller，这里不强制指定，保持与现有蓝图兼容。
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
