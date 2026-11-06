// Fill out your copyright notice in the Description page of Project Settings.

#include "Input/BlueBattleInputConfig.h"

#include "BlueBattleLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleInputConfig)

const UInputAction* UBlueBattleInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FBlueBattleInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogBlueBattle, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
