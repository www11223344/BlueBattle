// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "GameUIManagerSubsystem.h"

#include "BlueBattleUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UCommonActivatableWidget;
class ULocalPlayer;
class UObject;

UCLASS()
class UBlueBattleUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:

	UBlueBattleUIManagerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 把暂停菜单推入该玩家根布局的 UI.Layer.Menu；已打开或正在加载时忽略 */
	UFUNCTION(BlueprintCallable, Category = "BlueBattle|UI")
	void OpenPauseMenu(const ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintPure, Category = "BlueBattle|UI")
	bool IsPauseMenuOpen(const ULocalPlayer* LocalPlayer) const;

private:
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();

	UPROPERTY(Config)
	TSoftClassPtr<UCommonActivatableWidget> PauseMenuClass;

	/** 每个本地玩家当前的暂停菜单；值为空表示正在异步加载 */
	TMap<TWeakObjectPtr<const ULocalPlayer>, TWeakObjectPtr<UCommonActivatableWidget>> PauseMenus;
	
	FTSTicker::FDelegateHandle TickHandle;
};
