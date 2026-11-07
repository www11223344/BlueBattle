// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "View/MVVMViewModelContextResolver.h"

#include "BlueBattleLocalPlayerViewModelResolver.generated.h"

/**
 * MVVM Resolver：为 Widget 创建一个跟随其 Owning LocalPlayer 当前 Pawn 的 ViewModel（属性 / 爆气）。
 * 在 Widget 的 Viewmodels 面板中把 Creation Type 设为 Resolver，并选择本类即可。
 */
UCLASS(DisplayName = "BlueBattle Local Player Attribute Resolver")
class BLUEBATTLE_API UBlueBattleLocalPlayerViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
	virtual void DestroyInstance(UObject* ViewModel, const UMVVMView* View) const override;

#if WITH_EDITOR
	virtual bool DoesSupportViewModelClass(const UClass* Class) const override;
#endif
};
