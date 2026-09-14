// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlueBattleGameMode.generated.h"

class AWarriorCharacter;

/** 对局结束广播：胜者 index（0/1），-1 表示平局。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, int32, WinnerPlayerIndex);

/**
 * 本地双人对战 GameMode 骨架：负责监听角色死亡、判定胜负。
 * 表现/UI（胜利结算界面）仍由蓝图订阅事件处理。
 */
UCLASS()
class BLUEBATTLE_API ABlueBattleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABlueBattleGameMode();

	/** 对局结束事件（供结算 UI 订阅） */
	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnMatchEnded OnMatchEnded;

	/** 某个玩家角色死亡时调用（由角色/属性组件在死亡时通知） */
	UFUNCTION(BlueprintCallable, Category = "Match")
	void NotifyWarriorDied(int32 DeadPlayerIndex);

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMatchOver() const { return bMatchOver; }

protected:
	bool bMatchOver = false;
};
