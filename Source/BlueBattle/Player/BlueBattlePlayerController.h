// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonPlayerController.h"

#include "BlueBattlePlayerController.generated.h"

/**
 * 项目 PlayerController，继承 CommonGame 的 ACommonPlayerController（其父类为 AModularPlayerController），
 * 负责把 Controller/Pawn/PlayerState 的变化转发给 CommonLocalPlayer，供 UI 监听。
 * 关卡 UI 与输入映射由 Experience 的 GameFeatureAction 添加。
 */
UCLASS(Config = Game)
class BLUEBATTLE_API ABlueBattlePlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	ABlueBattlePlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
