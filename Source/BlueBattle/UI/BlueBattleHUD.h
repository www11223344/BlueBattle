// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"

#include "BlueBattleHUD.generated.h"

namespace EEndPlayReason { enum Type : int; }

/**
 * 对应 Lyra 的 ALyraHUD：本身不负责绘制，只把自己注册为 GameFrameworkComponent 接收者，
 * 以便 GameFeature / UIExtension 往 HUD 上挂组件。实际 HUD 布局通过 PrimaryGameLayout 的 Game 层推入。
 */
UCLASS(Config = Game)
class BLUEBATTLE_API ABlueBattleHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABlueBattleHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
