// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonGameInstance.h"

#include "BlueBattleGameInstance.generated.h"

/**
 * 项目 GameInstance，继承 CommonGame 的 UCommonGameInstance：
 * 本地玩家加入/移除时会通知 GameUIManagerSubsystem，由 UI Policy 为每个玩家创建 PrimaryGameLayout。
 * 蓝图 GameInstance 需 Reparent 到本类，否则 Lyra UI 层不会被创建。
 */
UCLASS(Config = Game)
class BLUEBATTLE_API UBlueBattleGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:
	UBlueBattleGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Init() override;
};
