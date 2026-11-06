// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BlueBattleUIFunctionLibrary.h"

#include "BlueBattleLogChannels.h"
#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagsManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Messaging/CommonGameDialog.h"
#include "Messaging/CommonMessagingSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleUIFunctionLibrary)

namespace BlueBattleUI
{
	static FGameplayTag ResolveLayerTag(FName Layer)
	{
		FString TagName = Layer.ToString();
		if (!TagName.StartsWith(TEXT("UI.Layer.")))
		{
			TagName = TEXT("UI.Layer.") + TagName;
		}
		return FGameplayTag::RequestGameplayTag(FName(*TagName), false);
	}
}

APlayerController* UBlueBattleUIFunctionLibrary::GetOwningPlayerController(UObject* WidgetContext)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(WidgetContext))
	{
		return PlayerController;
	}
	if (const UUserWidget* Widget = Cast<UUserWidget>(WidgetContext))
	{
		return Widget->GetOwningPlayer();
	}
	if (const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(WidgetContext))
	{
		return LocalPlayer->GetPlayerController(LocalPlayer->GetWorld());
	}
	return nullptr;
}

UCommonActivatableWidget* UBlueBattleUIFunctionLibrary::PushScreen(UObject* WidgetContext, FName Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	const APlayerController* PlayerController = GetOwningPlayerController(WidgetContext);
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	const FGameplayTag LayerTag = BlueBattleUI::ResolveLayerTag(Layer);
	if (!LocalPlayer || !LayerTag.IsValid() || !WidgetClass)
	{
		UE_LOG(LogBlueBattle, Warning, TEXT("PushScreen failed: Context=%s Layer=%s Class=%s"),
			*GetNameSafe(WidgetContext), *Layer.ToString(), *GetNameSafe(WidgetClass));
		return nullptr;
	}

	return UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, LayerTag, WidgetClass);
}

void UBlueBattleUIFunctionLibrary::RequestQuitGame(UObject* WidgetContext, FText Header, FText Body)
{
	APlayerController* PlayerController = GetOwningPlayerController(WidgetContext);
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	UCommonMessagingSubsystem* Messaging = LocalPlayer ? LocalPlayer->GetSubsystem<UCommonMessagingSubsystem>() : nullptr;
	if (!Messaging)
	{
		UKismetSystemLibrary::QuitGame(PlayerController, PlayerController, EQuitPreference::Quit, false);
		return;
	}

	TWeakObjectPtr<APlayerController> WeakPlayerController = PlayerController;
	Messaging->ShowConfirmation(UCommonGameDialogDescriptor::CreateConfirmationYesNo(Header, Body),
		FCommonMessagingResultDelegate::CreateLambda([WeakPlayerController](ECommonMessagingResult Result)
		{
			if (Result == ECommonMessagingResult::Confirmed && WeakPlayerController.IsValid())
			{
				UKismetSystemLibrary::QuitGame(WeakPlayerController.Get(), WeakPlayerController.Get(), EQuitPreference::Quit, false);
			}
		}));
}
