// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CommonUserWidget.h"
#include "UI/BlueBattleActivatableWidget.h"

#include "BlueBattleDebugWindow.generated.h"

class UEditableTextBox;
class UTextBlock;

/** 调试窗口中的一条指令：每方各显示一行，执行时发出控制台命令 "<Command> <Side> <Value>"。 */
USTRUCT(BlueprintType)
struct FBlueBattleDebugCommandEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	FString Command;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	FString DefaultValue;

	/** 值输入框为空时显示的提示 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	FText ValueHint;
};

DECLARE_DELEGATE_OneParam(FOnBlueBattleDebugCommandExecuted, const FString& /*Result*/);

/** 调试窗口的一行：名称 / 数值 / 执行按钮，作用于固定的一方。控件树在 C++ 中构建，无需蓝图资产。 */
UCLASS()
class BLUEBATTLE_API UBlueBattleDebugCommandRow : public UUserWidget
{
	GENERATED_BODY()

public:
	/** InSide：0 = P1，1 = P2 */
	void Setup(const FBlueBattleDebugCommandEntry& InEntry, int32 InSide);

	FOnBlueBattleDebugCommandExecuted OnExecuted;

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleExecuteClicked();

	FBlueBattleDebugCommandEntry Entry;

	int32 Side = 0;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(Transient)
	TObjectPtr<UEditableTextBox> ValueTextBox;
};

/**
 * 局内调试窗口：作为 UBlueBattleDebugHUDEntry 的子控件挂在 HUD 布局的 HUD.Slot.Debug 插槽中，默认未激活（隐藏）。
 * 激活期间显示鼠标，游戏输入不受影响；Esc / 关闭按钮 / 再按 K 关闭。
 * P1 / P2 各一栏，实时显示该方生命值与伤害设置，下面是该方的指令行；
 * 新增指令时注册对应的 BB.* 控制台命令，再在 Commands 中加一项（两栏都会出现）。
 */
UCLASS()
class BLUEBATTLE_API UBlueBattleDebugWindow : public UBlueBattleActivatableWidget
{
	GENERATED_BODY()

public:
	UBlueBattleDebugWindow(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TArray<FBlueBattleDebugCommandEntry> Commands;

	UPROPERTY(EditDefaultsOnly, Category = "Debug", meta = (ClampMin = 0))
	float StatusRefreshInterval = 0.2f;

private:
	UFUNCTION()
	void HandleCloseClicked();

	void HandleCommandExecuted(const FString& Result);
	void RefreshStatus();

	/** 按 Side 索引的状态行 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTextBlock>> StatusTexts;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResultText;

	float StatusRefreshCountdown = 0.f;
};

/**
 * 调试窗口的 HUD 插槽入口：由 Experience 的 AddWidgets 动作注册到 HUD.Slot.Debug（与 Lyra 的 HUD 元素相同的扩展方式）。
 * 本身不是可激活控件，按键绑定归属 HUD 布局，按 UI.Action.DebugWindow（默认 K）开关内部的调试窗口。Shipping 下不创建窗口。
 */
UCLASS()
class BLUEBATTLE_API UBlueBattleDebugHUDEntry : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UBlueBattleDebugHUDEntry(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TSubclassOf<UBlueBattleDebugWindow> DebugWindowClass;

private:
	void HandleToggleAction();

	UPROPERTY(Transient)
	TObjectPtr<UBlueBattleDebugWindow> DebugWindow;
};
