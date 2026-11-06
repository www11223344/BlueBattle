// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Foundation/BlueBattleLoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleLoadingScreenSubsystem)

class UUserWidget;

//////////////////////////////////////////////////////////////////////
// UBlueBattleLoadingScreenSubsystem

UBlueBattleLoadingScreenSubsystem::UBlueBattleLoadingScreenSubsystem()
{
}

void UBlueBattleLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (LoadingScreenWidgetClass != NewWidgetClass)
	{
		LoadingScreenWidgetClass = NewWidgetClass;

		OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
	}
}

TSubclassOf<UUserWidget> UBlueBattleLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}

