// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"

#include "BlueBattlePawnData.generated.h"

class APawn;
class UBlueBattleInputConfig;

/** 描述玩家 Pawn 的不可变数据（对应 Lyra 的 ULyraPawnData，去掉了 GAS/相机部分） */
UCLASS(BlueprintType, Const, meta = (DisplayName = "BlueBattle Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class BLUEBATTLE_API UBlueBattlePawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
	TSubclassOf<APawn> PawnClass;

	/** 由 HeroComponent 用来把输入动作绑定到输入标签 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UBlueBattleInputConfig> InputConfig;
};
