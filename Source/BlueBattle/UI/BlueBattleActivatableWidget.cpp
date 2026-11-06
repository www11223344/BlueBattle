// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueBattleActivatableWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Editor/WidgetCompilerLog.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIInputTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleActivatableWidget)

#define LOCTEXT_NAMESPACE "BlueBattle"

UBlueBattleActivatableWidget::UBlueBattleActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TOptional<FUIInputConfig> UBlueBattleActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EBlueBattleWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EBlueBattleWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EBlueBattleWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EBlueBattleWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

UWidget* UBlueBattleActivatableWidget::NativeGetDesiredFocusTarget() const
{
	if (UWidget* BlueprintTarget = Super::NativeGetDesiredFocusTarget())
	{
		return BlueprintTarget;
	}
	return (!DesiredFocusWidgetName.IsNone() && WidgetTree) ? WidgetTree->FindWidget(DesiredFocusWidgetName) : nullptr;
}

void UBlueBattleActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (bPauseGameWhileActive)
	{
		if (APlayerController* PlayerController = GetOwningPlayer(); PlayerController && !PlayerController->IsPaused())
		{
			bPausedGame = PlayerController->SetPause(true);
		}
	}
}

void UBlueBattleActivatableWidget::NativeOnDeactivated()
{
	if (bPausedGame)
	{
		bPausedGame = false;
		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			PlayerController->SetPause(false);
		}
	}

	Super::NativeOnDeactivated();
}

void UBlueBattleActivatableWidget::ListenForUIAction(FDataTableRowHandle InputAction, FName ActionId, bool bShowInActionBar)
{
	FBindUIActionArgs Args(InputAction, bShowInActionBar, FSimpleDelegate::CreateWeakLambda(this, [this, ActionId]()
	{
		OnUIAction(ActionId);
	}));
	ListenedActionHandles.Add(RegisterUIActionBinding(Args));
}

void UBlueBattleActivatableWidget::StopListeningForUIActions()
{
	for (FUIActionBindingHandle& Handle : ListenedActionHandles)
	{
		Handle.Unregister();
	}
	ListenedActionHandles.Reset();
}

#if WITH_EDITOR

void UBlueBattleActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!DesiredFocusWidgetName.IsNone())
	{
		if (!BlueprintWidgetTree.FindWidget(DesiredFocusWidgetName))
		{
			CompileLog.Warning(FText::Format(LOCTEXT("DesiredFocusWidgetMissing", "DesiredFocusWidgetName '{0}' was not found in the widget tree."), FText::FromName(DesiredFocusWidgetName)));
		}
		return;
	}

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UBlueBattleActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UBlueBattleActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE
