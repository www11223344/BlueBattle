// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ModularGameMode.h"
#include "BlueBattleGameMode.generated.h"

class AWarriorCharacter;
class APlayerController;
class UCommonActivatableWidget;
class UTexture2D;

/** 对局结束广播：胜者 index（0/1），-1 表示平局。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, int32, WinnerPlayerIndex);

/** 某位玩家获胜时结算界面显示的内容 */
USTRUCT(BlueprintType)
struct FBlueBattleWinnerDisplay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	FText WinnerText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	TSoftObjectPtr<UTexture2D> WinnerLogo;
};

/**
 * 本地双人对战 GameMode：负责监听角色死亡、判定胜负，并为每个本地玩家推送该关卡的 UI。
 *  - HUDLayoutClass：推到每个本地玩家的 UI.Layer.Game（局内 HUD 布局，处理 Esc 暂停菜单）
 *  - InitialScreenClass：推到首个本地玩家的 InitialScreenLayer（前端关卡的开场/主菜单）
 *  - MatchResultScreenClass：对局结束后推到首个本地玩家的 UI.Layer.Menu（结算界面）
 */
UCLASS()
class BLUEBATTLE_API ABlueBattleGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	ABlueBattleGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** 对局结束事件（供结算 UI 订阅） */
	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnMatchEnded OnMatchEnded;

	/** 某个玩家角色死亡时调用（由角色/属性组件在死亡时通知） */
	UFUNCTION(BlueprintCallable, Category = "Match")
	void NotifyWarriorDied(int32 DeadPlayerIndex);

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMatchOver() const { return bMatchOver; }

	/** 本地玩家的 PrimaryGameLayout 就绪后由 PlayerController 调用，推送本关卡的 UI */
	void InitializeLocalPlayerUI(APlayerController* PlayerController);

protected:
	virtual void BeginPlay() override;

	void ShowMatchResult(int32 WinnerPlayerIndex);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftClassPtr<UCommonActivatableWidget> HUDLayoutClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftClassPtr<UCommonActivatableWidget> InitialScreenClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag InitialScreenLayer;

	UPROPERTY(EditDefaultsOnly, Category = "Match|UI")
	TSoftClassPtr<UCommonActivatableWidget> MatchResultScreenClass;

	/** 死亡到弹出结算界面的间隔（留给死亡动画） */
	UPROPERTY(EditDefaultsOnly, Category = "Match|UI", meta = (ClampMin = 0))
	float MatchResultScreenDelay = 3.f;

	/** 按胜者本地玩家序号索引（0 = P1 获胜，1 = P2 获胜） */
	UPROPERTY(EditDefaultsOnly, Category = "Match|UI")
	TArray<FBlueBattleWinnerDisplay> WinnerDisplays;

	bool bMatchOver = false;

private:
	FTimerHandle MatchResultTimerHandle;
};
