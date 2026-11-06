// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonPlayerController.h"

#include "BlueBattlePlayerController.generated.h"

/**
 * 项目 PlayerController，继承 CommonGame 的 ACommonPlayerController（其父类为 AModularPlayerController），
 * 负责把 Controller/Pawn/PlayerState 的变化转发给 CommonLocalPlayer，供 UI 监听。
 * 本地玩家的 PrimaryGameLayout 就绪后，请求 GameMode 推送本关卡的 UI。
 */
UCLASS(Config = Game)
class BLUEBATTLE_API ABlueBattlePlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	ABlueBattlePlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ReceivedPlayer() override;

protected:
	virtual void BeginPlay() override;

private:
	/** 初始玩家先 ReceivedPlayer 后 BeginPlay，中途加入的玩家相反；两者都满足后再推 UI */
	void TryInitializeLocalPlayerUI();

	bool bLocalPlayerUIInitialized = false;
};
