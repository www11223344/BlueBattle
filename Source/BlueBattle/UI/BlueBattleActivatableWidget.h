// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "Engine/DataTable.h"
#include "Input/UIActionBindingHandle.h"

#include "BlueBattleActivatableWidget.generated.h"

struct FUIInputConfig;

UENUM(BlueprintType)
enum class EBlueBattleWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};

// An activatable widget that automatically drives the desired input config when activated
UCLASS(Abstract, Blueprintable)
class BLUEBATTLE_API UBlueBattleActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UBlueBattleActivatableWidget(const FObjectInitializer& ObjectInitializer);
	
public:
	
	//~UCommonActivatableWidget interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

	/** 监听一个 CommonUI 输入动作（如 DT_UniversalActions 的 Confirm），触发时调用 OnUIAction(ActionId) */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|UI")
	void ListenForUIAction(FDataTableRowHandle InputAction, FName ActionId, bool bShowInActionBar = false);

	UFUNCTION(BlueprintCallable, Category = "BlueBattle|UI")
	void StopListeningForUIActions();

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const override;
#endif
	
protected:
	//~UCommonActivatableWidget interface
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	//~End of UCommonActivatableWidget interface

	/** ListenForUIAction 注册的动作被触发 */
	UFUNCTION(BlueprintImplementableEvent, Category = "BlueBattle|UI")
	void OnUIAction(FName ActionId);

	/** The desired input mode to use while this UI is activated, for example do you want key presses to still reach the game/player controller? */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EBlueBattleWidgetInputMode InputConfig = EBlueBattleWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;

	/** 激活时获得焦点的子控件名（手柄导航起点）；蓝图实现了 GetDesiredFocusTarget 时以蓝图为准 */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	FName DesiredFocusWidgetName;

	/** 激活期间暂停游戏（暂停菜单） */
	UPROPERTY(EditDefaultsOnly, Category = "BlueBattle|UI")
	bool bPauseGameWhileActive = false;

private:
	TArray<FUIActionBindingHandle> ListenedActionHandles;
	bool bPausedGame = false;
};
