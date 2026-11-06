// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModels/BlueBattleMatchResultViewModel.h"

#include "Engine/Texture2D.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleMatchResultViewModel)

void UBlueBattleMatchResultViewModel::SetMatchResult(int32 InWinnerPlayerIndex, const FText& InWinnerText, UTexture2D* InWinnerLogo)
{
	UE_MVVM_SET_PROPERTY_VALUE(WinnerPlayerIndex, InWinnerPlayerIndex);
	UE_MVVM_SET_PROPERTY_VALUE(WinnerText, InWinnerText);
	UE_MVVM_SET_PROPERTY_VALUE(WinnerLogo, InWinnerLogo);
	UE_MVVM_SET_PROPERTY_VALUE(bMatchOver, true);
}

void UBlueBattleMatchResultViewModel::Reset()
{
	UE_MVVM_SET_PROPERTY_VALUE(bMatchOver, false);
	UE_MVVM_SET_PROPERTY_VALUE(WinnerPlayerIndex, INDEX_NONE);
	UE_MVVM_SET_PROPERTY_VALUE(WinnerText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(WinnerLogo, nullptr);
}
