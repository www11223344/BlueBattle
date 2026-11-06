// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonLocalPlayer.h"

#include "BlueBattleLocalPlayer.generated.h"

/**
 * 项目 LocalPlayer。分屏双人时每个本地玩家各有一个实例，并各自拥有一个 PrimaryGameLayout。
 */
UCLASS()
class BLUEBATTLE_API UBlueBattleLocalPlayer : public UCommonLocalPlayer
{
	GENERATED_BODY()
};
