// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "BlueBattleInputConfig.generated.h"

class UInputAction;

USTRUCT(BlueprintType)
struct FBlueBattleInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/** 输入动作与输入标签的对应表（对应 Lyra 的 ULyraInputConfig） */
UCLASS(BlueprintType, Const)
class BLUEBATTLE_API UBlueBattleInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	/** 由 HeroComponent 逐个绑定到固定处理函数（移动、视角等） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputAction"))
	TArray<FBlueBattleInputAction> NativeInputActions;

	/** 按下时把输入标签转发给 Pawn（Lyra 中这里对应 AbilityInputActions） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputAction"))
	TArray<FBlueBattleInputAction> ActionInputActions;
};
