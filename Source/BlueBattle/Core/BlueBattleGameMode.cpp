// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BlueBattleGameMode.h"
#include "BlueBattleGameplayTags.h"
#include "BlueBattleLogChannels.h"
#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "Core/BlueBattleGameState.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Player/BlueBattlePlayerController.h"
#include "Player/BlueBattlePlayerState.h"
#include "TimerManager.h"
#include "UI/BlueBattleHUD.h"
#include "UI/ViewModels/BlueBattleMatchResultViewModel.h"

ABlueBattleGameMode::ABlueBattleGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Pawn 仍由蓝图 GameMode 指定；蓝图子类中显式设置过的类不受这里影响。
	GameStateClass = ABlueBattleGameState::StaticClass();
	PlayerControllerClass = ABlueBattlePlayerController::StaticClass();
	PlayerStateClass = ABlueBattlePlayerState::StaticClass();
	HUDClass = ABlueBattleHUD::StaticClass();

	InitialScreenLayer = BlueBattleGameplayTags::UI_Layer_Menu;
}

void ABlueBattleGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 上一局结算时关闭了分屏，新关卡恢复
	if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
	{
		ViewportClient->SetForceDisableSplitscreen(false);
	}
}

void ABlueBattleGameMode::InitializeLocalPlayerUI(APlayerController* PlayerController)
{
	ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (!LocalPlayer)
	{
		return;
	}

	if (TSubclassOf<UCommonActivatableWidget> HUDLayout = HUDLayoutClass.LoadSynchronous())
	{
		UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, BlueBattleGameplayTags::UI_Layer_Game, HUDLayout);
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const bool bIsPrimaryPlayer = GameInstance && GameInstance->GetFirstGamePlayer() == LocalPlayer;
	if (bIsPrimaryPlayer)
	{
		if (TSubclassOf<UCommonActivatableWidget> InitialScreen = InitialScreenClass.LoadSynchronous())
		{
			UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, InitialScreenLayer, InitialScreen);
		}
	}
}

void ABlueBattleGameMode::NotifyWarriorDied(int32 DeadPlayerIndex)
{
	if (bMatchOver)
	{
		return;
	}

	bMatchOver = true;

	// 双人对战：一方死亡则另一方获胜（0 <-> 1）
	const int32 Winner = (DeadPlayerIndex == 0) ? 1 : 0;
	UE_LOG(LogBlueBattle, Log, TEXT("Match over: player %d died, player %d wins"), DeadPlayerIndex, Winner);
	OnMatchEnded.Broadcast(Winner);

	GetWorldTimerManager().SetTimer(MatchResultTimerHandle,
		FTimerDelegate::CreateUObject(this, &ThisClass::ShowMatchResult, Winner),
		FMath::Max(MatchResultScreenDelay, UE_KINDA_SMALL_NUMBER), false);
}

void ABlueBattleGameMode::ShowMatchResult(int32 WinnerPlayerIndex)
{
	FText WinnerText;
	FSlateBrush WinnerLogo;
	if (WinnerDisplays.IsValidIndex(WinnerPlayerIndex))
	{
		const FBlueBattleWinnerDisplay& Display = WinnerDisplays[WinnerPlayerIndex];
		WinnerText = Display.WinnerText;
		if (UTexture2D* Logo = Display.WinnerLogo.LoadSynchronous())
		{
			WinnerLogo.SetResourceObject(Logo);
			WinnerLogo.ImageSize = FVector2D(Logo->GetSizeX(), Logo->GetSizeY());
		}
	}

	if (ABlueBattleGameState* BlueBattleGameState = GetGameState<ABlueBattleGameState>())
	{
		BlueBattleGameState->GetMatchResultViewModel()->SetMatchResult(WinnerPlayerIndex, WinnerText, WinnerLogo);
	}

	// 结算界面全屏显示在 P1 的布局上
	if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
	{
		ViewportClient->SetForceDisableSplitscreen(true);
	}

	const UGameInstance* GameInstance = GetGameInstance();
	ULocalPlayer* PrimaryPlayer = GameInstance ? GameInstance->GetFirstGamePlayer() : nullptr;
	if (TSubclassOf<UCommonActivatableWidget> ResultScreen = MatchResultScreenClass.LoadSynchronous(); ResultScreen && PrimaryPlayer)
	{
		UCommonUIExtensions::PushContentToLayer_ForPlayer(PrimaryPlayer, BlueBattleGameplayTags::UI_Layer_Menu, ResultScreen);
	}
}
