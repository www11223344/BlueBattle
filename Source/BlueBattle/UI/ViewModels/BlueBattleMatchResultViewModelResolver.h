// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "View/MVVMViewModelContextResolver.h"

#include "BlueBattleMatchResultViewModelResolver.generated.h"

/**
 * MVVM Resolver：返回当前世界 GameState 上的对局结算 ViewModel（全局共享一份）。
 */
UCLASS(DisplayName = "BlueBattle Match Result Resolver")
class BLUEBATTLE_API UBlueBattleMatchResultViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;

#if WITH_EDITOR
	virtual bool DoesSupportViewModelClass(const UClass* Class) const override;
#endif
};
