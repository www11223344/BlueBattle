// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MVVMViewModelBase.h"
#include "Styling/SlateBrush.h"

#include "BlueBattleMatchResultViewModel.generated.h"

/**
 * 对局结算 ViewModel（MVVM）：由 GameMode 在对局结束时写入胜者信息，结算界面绑定字段显示。
 * 实例挂在 GameState 上，结算 Widget 通过 BlueBattleMatchResultViewModelResolver 取得。
 */
UCLASS(BlueprintType, DisplayName = "BlueBattle Match Result Viewmodel")
class BLUEBATTLE_API UBlueBattleMatchResultViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void SetMatchResult(int32 InWinnerPlayerIndex, const FText& InWinnerText, const FSlateBrush& InWinnerLogo);
	void Reset();

	bool IsMatchOver() const { return bMatchOver; }
	int32 GetWinnerPlayerIndex() const { return WinnerPlayerIndex; }
	const FText& GetWinnerText() const { return WinnerText; }
	const FSlateBrush& GetWinnerLogo() const { return WinnerLogo; }

protected:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter = "IsMatchOver", Category = "Match")
	bool bMatchOver = false;

	/** 胜者本地玩家序号（0 = P1，1 = P2），-1 表示未决出 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Match")
	int32 WinnerPlayerIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Match")
	FText WinnerText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Match")
	FSlateBrush WinnerLogo;
};
