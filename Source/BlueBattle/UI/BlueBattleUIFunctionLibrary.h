// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BlueBattleUIFunctionLibrary.generated.h"

class APlayerController;
class UCommonActivatableWidget;
class UObject;

/**
 * 供 UnLua / 蓝图调用的 UI 流程工具：把界面推到玩家的 PrimaryGameLayout 层、弹确认框等。
 * Layer 参数可写完整 Tag（UI.Layer.Menu）或简写（Menu / Game / GameMenu / Modal）。
 */
UCLASS()
class BLUEBATTLE_API UBlueBattleUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|UI", meta = (DefaultToSelf = "WidgetContext"))
	static UCommonActivatableWidget* PushScreen(UObject* WidgetContext, FName Layer, TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/** 弹出"是/否"确认框，确认后退出游戏 */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|UI", meta = (DefaultToSelf = "WidgetContext"))
	static void RequestQuitGame(UObject* WidgetContext, FText Header, FText Body);

	/** 从 Widget / PlayerController / LocalPlayer 解析出所属 PlayerController */
	UFUNCTION(BlueprintPure, Category = "BlueBattle|UI", meta = (DefaultToSelf = "WidgetContext"))
	static APlayerController* GetOwningPlayerController(UObject* WidgetContext);
};
