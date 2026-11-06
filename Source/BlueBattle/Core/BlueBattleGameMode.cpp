// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/BlueBattleGameMode.h"
#include "BlueBattleGameplayTags.h"
#include "BlueBattleLogChannels.h"
#include "Characters/BlueBattlePawnData.h"
#include "Characters/BlueBattlePawnExtensionComponent.h"
#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "Core/BlueBattleGameState.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "GameModes/BlueBattleExperienceDefinition.h"
#include "GameModes/BlueBattleExperienceManagerComponent.h"
#include "GameModes/BlueBattleWorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Player/BlueBattlePlayerController.h"
#include "Player/BlueBattlePlayerState.h"
#include "TimerManager.h"
#include "UI/BlueBattleHUD.h"
#include "UI/ViewModels/BlueBattleMatchResultViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleGameMode)

ABlueBattleGameMode::ABlueBattleGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ABlueBattleGameState::StaticClass();
	PlayerControllerClass = ABlueBattlePlayerController::StaticClass();
	PlayerStateClass = ABlueBattlePlayerState::StaticClass();
	HUDClass = ABlueBattleHUD::StaticClass();
}

void ABlueBattleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// WorldSettings 等在 InitGame 时还未就绪，下一帧再选定 Experience
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

void ABlueBattleGameMode::InitGameState()
{
	Super::InitGameState();

	UBlueBattleExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UBlueBattleExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnBlueBattleExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
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

void ABlueBattleGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
	const FPrimaryAssetType ExperienceType(UBlueBattleExperienceDefinition::StaticClass()->GetFName());
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	if (UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(ExperienceType, FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}

	if (!ExperienceId.IsValid())
	{
		FString ExperienceFromCommandLine;
		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
		{
			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
			if (!ExperienceId.PrimaryAssetType.IsValid())
			{
				ExperienceId = FPrimaryAssetId(ExperienceType, FName(*ExperienceFromCommandLine));
			}
			ExperienceIdSource = TEXT("CommandLine");
		}
	}

	if (!ExperienceId.IsValid())
	{
		if (const ABlueBattleWorldSettings* TypedWorldSettings = Cast<ABlueBattleWorldSettings>(GetWorldSettings()))
		{
			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
			ExperienceIdSource = TEXT("WorldSettings");
		}
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetPath(ExperienceId).IsValid())
	{
		UE_LOG(LogBlueBattleExperience, Error, TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default"), *ExperienceId.ToString());
		ExperienceId = FPrimaryAssetId();
	}

	if (!ExperienceId.IsValid() && !DefaultExperience.IsNull())
	{
		ExperienceId = AssetManager.GetPrimaryAssetIdForPath(DefaultExperience.ToSoftObjectPath());
		ExperienceIdSource = TEXT("Default");
	}

	if (!ExperienceId.IsValid())
	{
		UE_LOG(LogBlueBattleExperience, Error, TEXT("EXPERIENCE: No experience found for %s, players will not spawn"), *GetNameSafe(GetWorld()));
		return;
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

void ABlueBattleGameMode::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
	UE_LOG(LogBlueBattleExperience, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(), *ExperienceIdSource);

	UBlueBattleExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UBlueBattleExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->SetCurrentExperience(ExperienceId);
}

bool ABlueBattleGameMode::IsExperienceLoaded() const
{
	const UBlueBattleExperienceManagerComponent* ExperienceComponent = GameState ? GameState->FindComponentByClass<UBlueBattleExperienceManagerComponent>() : nullptr;
	return ExperienceComponent && ExperienceComponent->IsExperienceLoaded();
}

void ABlueBattleGameMode::OnExperienceLoaded(const UBlueBattleExperienceDefinition* CurrentExperience)
{
	RemoveExtraLocalPlayers(CurrentExperience->NumLocalPlayers);

	// 加载期间登录的玩家被推迟了生成，现在补上
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->GetPawn() == nullptr && PlayerCanRestart(PC))
		{
			RestartPlayer(PC);
		}
	}

	// 已有玩家生成后再创建，新玩家登录时才能避开被占用的 PlayerStart
	CreateMissingLocalPlayers(CurrentExperience->NumLocalPlayers);
}

void ABlueBattleGameMode::RemoveExtraLocalPlayers(int32 NumLocalPlayers)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const ULocalPlayer* PrimaryPlayer = GameInstance->GetFirstGamePlayer();
	const TArray<ULocalPlayer*> LocalPlayers = GameInstance->GetLocalPlayers();
	int32 NumKept = PrimaryPlayer ? 1 : 0;
	for (ULocalPlayer* LocalPlayer : LocalPlayers)
	{
		if (!LocalPlayer || LocalPlayer == PrimaryPlayer)
		{
			continue;
		}

		if (NumKept < NumLocalPlayers)
		{
			++NumKept;
			continue;
		}

		UE_LOG(LogBlueBattle, Log, TEXT("Removing extra local player %s"), *GetNameSafe(LocalPlayer));
		GameInstance->RemoveLocalPlayer(LocalPlayer);
	}
}

void ABlueBattleGameMode::CreateMissingLocalPlayers(int32 NumLocalPlayers)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	while (GameInstance->GetNumLocalPlayers() < NumLocalPlayers)
	{
		FString Error;
		if (!GameInstance->CreateLocalPlayer(INDEX_NONE, Error, true))
		{
			UE_LOG(LogBlueBattle, Error, TEXT("Failed to create local player: %s"), *Error);
			break;
		}
	}
}

void ABlueBattleGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Experience 加载完成前不生成 Pawn（OnExperienceLoaded 会补上）
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

const UBlueBattlePawnData* ABlueBattleGameMode::GetPawnDataForController(const AController* InController) const
{
	if (!IsExperienceLoaded())
	{
		return nullptr;
	}

	const UBlueBattleExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UBlueBattleExperienceManagerComponent>();
	return ExperienceComponent->GetCurrentExperienceChecked()->DefaultPawnData;
}

UClass* ABlueBattleGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UBlueBattlePawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ABlueBattleGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true;

	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		return nullptr;
	}

	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
	if (!SpawnedPawn)
	{
		UE_LOG(LogBlueBattle, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		return nullptr;
	}

	if (UBlueBattlePawnExtensionComponent* PawnExtComp = UBlueBattlePawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
	{
		if (const UBlueBattlePawnData* PawnData = GetPawnDataForController(NewPlayer))
		{
			PawnExtComp->SetPawnData(PawnData);
		}
	}

	SpawnedPawn->FinishSpawning(SpawnTransform);
	return SpawnedPawn;
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
	UTexture2D* WinnerLogo = nullptr;
	if (WinnerDisplays.IsValidIndex(WinnerPlayerIndex))
	{
		const FBlueBattleWinnerDisplay& Display = WinnerDisplays[WinnerPlayerIndex];
		WinnerText = Display.WinnerText;
		WinnerLogo = Display.WinnerLogo.LoadSynchronous();
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
