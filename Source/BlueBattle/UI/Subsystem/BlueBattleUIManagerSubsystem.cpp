// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueBattleUIManagerSubsystem.h"

#include "BlueBattleGameplayTags.h"
#include "CommonActivatableWidget.h"
#include "CommonLocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"
#include "GameUIPolicy.h"
#include "PrimaryGameLayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleUIManagerSubsystem)

class FSubsystemCollectionBase;

UBlueBattleUIManagerSubsystem::UBlueBattleUIManagerSubsystem()
{
}

void UBlueBattleUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UBlueBattleUIManagerSubsystem::Tick), 0.0f);
}

void UBlueBattleUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

void UBlueBattleUIManagerSubsystem::OpenPauseMenu(const ULocalPlayer* LocalPlayer)
{
	const UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer);
	const UGameUIPolicy* Policy = GetCurrentUIPolicy();
	UPrimaryGameLayout* RootLayout = Policy && CommonLocalPlayer ? Policy->GetRootLayout(CommonLocalPlayer) : nullptr;
	if (!RootLayout || !ensure(!PauseMenuClass.IsNull()) || IsPauseMenuOpen(LocalPlayer))
	{
		return;
	}

	const TWeakObjectPtr<const ULocalPlayer> PlayerKey(LocalPlayer);
	PauseMenus.Add(PlayerKey, nullptr);

	const bool bSuspendInputUntilComplete = true;
	RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(BlueBattleGameplayTags::UI_Layer_Menu, bSuspendInputUntilComplete, PauseMenuClass,
		[this, PlayerKey](EAsyncWidgetLayerState State, UCommonActivatableWidget* Widget)
		{
			if (State == EAsyncWidgetLayerState::AfterPush && Widget)
			{
				PauseMenus.Add(PlayerKey, Widget);
			}
			else if (State == EAsyncWidgetLayerState::Canceled)
			{
				PauseMenus.Remove(PlayerKey);
			}
		});
}

bool UBlueBattleUIManagerSubsystem::IsPauseMenuOpen(const ULocalPlayer* LocalPlayer) const
{
	const TWeakObjectPtr<UCommonActivatableWidget>* PauseMenu = PauseMenus.Find(LocalPlayer);
	if (!PauseMenu)
	{
		return false;
	}
	if (PauseMenu->IsExplicitlyNull())
	{
		return true;
	}
	const UCommonActivatableWidget* Widget = PauseMenu->Get();
	return Widget && Widget->IsActivated();
}

bool UBlueBattleUIManagerSubsystem::Tick(float DeltaTime)
{
	SyncRootLayoutVisibilityToShowHUD();
	
	return true;
}

void UBlueBattleUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD()
{
	if (const UGameUIPolicy* Policy = GetCurrentUIPolicy())
	{
		for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
		{
			bool bShouldShowUI = true;
			
			if (const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				const AHUD* HUD = PC->GetHUD();

				if (HUD && !HUD->bShowHUD)
				{
					bShouldShowUI = false;
				}
			}

			if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer)))
			{
				const ESlateVisibility DesiredVisibility = bShouldShowUI ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
				if (DesiredVisibility != RootLayout->GetVisibility())
				{
					RootLayout->SetVisibility(DesiredVisibility);	
				}
			}
		}
	}
}
