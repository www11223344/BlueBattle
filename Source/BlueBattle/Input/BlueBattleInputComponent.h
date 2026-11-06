// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnhancedInputComponent.h"
#include "Input/BlueBattleInputConfig.h"

#include "BlueBattleInputComponent.generated.h"

/** 按 InputConfig 里的输入标签绑定输入动作（对应 Lyra 的 ULyraInputComponent） */
UCLASS(Config = Input)
class BLUEBATTLE_API UBlueBattleInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	template <class UserClass, typename FuncType>
	void BindNativeAction(const UBlueBattleInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template <class UserClass, typename PressedFuncType>
	void BindActionInputs(const UBlueBattleInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UBlueBattleInputComponent::BindNativeAction(const UBlueBattleInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template <class UserClass, typename PressedFuncType>
void UBlueBattleInputComponent::BindActionInputs(const UBlueBattleInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FBlueBattleInputAction& Action : InputConfig->ActionInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			// 动作没有配置 Trigger 时 Triggered 会每帧触发，按下只响应一次用 Started
			BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Started, Object, PressedFunc, Action.InputTag).GetHandle());
		}
	}
}
