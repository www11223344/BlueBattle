// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "BlueBattleGameMode.generated.h"

class AController;
class UBlueBattleExperienceDefinition;
class UBlueBattlePawnData;
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
 * 项目 GameMode（对应 Lyra 的 ALyraGameMode，去掉了 GAS 与联机匹配部分）：
 *  - 启动时选定 Experience（URL 选项 > 命令行 > WorldSettings > DefaultExperience），加载完成前不生成 Pawn
 *  - Experience 加载后按其 NumLocalPlayers 增删本地玩家，Pawn 类与 PawnData 取自 Experience
 *  - 监听角色死亡、判定胜负并推送结算界面
 */
UCLASS()
class BLUEBATTLE_API ABlueBattleGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	ABlueBattleGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	//~ End AGameModeBase interface

	UFUNCTION(BlueprintCallable, Category = "BlueBattle|Pawn")
	const UBlueBattlePawnData* GetPawnDataForController(const AController* InController) const;

	/** 对局结束事件（供结算 UI 订阅） */
	UPROPERTY(BlueprintAssignable, Category = "Match")
	FOnMatchEnded OnMatchEnded;

	/** 某个玩家角色死亡时调用（由角色/属性组件在死亡时通知） */
	UFUNCTION(BlueprintCallable, Category = "Match")
	void NotifyWarriorDied(int32 DeadPlayerIndex);

	UFUNCTION(BlueprintPure, Category = "Match")
	bool IsMatchOver() const { return bMatchOver; }

protected:
	virtual void BeginPlay() override;

	void ShowMatchResult(int32 WinnerPlayerIndex);

	/** URL、命令行、WorldSettings 都未指定时使用的 Experience */
	UPROPERTY(EditDefaultsOnly, Category = "Experience")
	TSoftObjectPtr<UBlueBattleExperienceDefinition> DefaultExperience;

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
	void HandleMatchAssignmentIfNotExpectingOne();
	void OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);
	void OnExperienceLoaded(const UBlueBattleExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;

	/** 让本地玩家数量与 Experience 一致（P2 回到菜单时移除，进入对战时创建） */
	void ApplyNumLocalPlayers(int32 NumLocalPlayers);

	FTimerHandle MatchResultTimerHandle;
};
