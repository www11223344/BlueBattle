// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BlueBattleActivatableWidget.h"
#include "Containers/Ticker.h"
#include "GameplayTagContainer.h"

#include "BlueBattleHUDLayout.generated.h"

class UCommonActivatableWidget;
class UObject;
class UWidget;
class UBlueBattleControllerDisconnectedScreen;

/**
 * UBlueBattleHUDLayout
 *
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "BlueBattle HUD Layout", Category = "BlueBattle|HUD"))
class UBlueBattleHUDLayout : public UBlueBattleActivatableWidget
{
	GENERATED_BODY()

public:

	UBlueBattleHUDLayout(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	void HandleEscapeAction();

	/** 根布局覆盖整个视口时，把 PlayerScreenRegion 收缩到所属本地玩家的分屏区域 */
	void UpdatePlayerScreenRegion(const FGeometry& MyGeometry);

	/** 只覆盖所属玩家分屏区域的容器（须放在 Overlay 中），放置每个玩家各自的 HUD 元素插槽 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PlayerScreenRegion;
	
	/** 
	* Callback for when controllers are disconnected. This will check if the player now has 
	* no mapped input devices to them, which would mean that they can't play the game.
	* 
	* If this is the case, then call DisplayControllerDisconnectedMenu.
	*/
	void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);

	/**
	* Callback for when controllers change their owning platform user. We will use this to check
	* if we no longer need to display the "Controller Disconnected" menu
	*/
	void HandleInputDevicePairingChanged(FInputDeviceId InputDeviceId, FPlatformUserId NewUserPlatformId, FPlatformUserId OldUserPlatformId);
	
	/**
	* Notify this widget that the state of controllers for the player have changed. Queue a timer for next tick to 
	* process them and see if we need to show/hide the "controller disconnected" widget.
	*/
	void NotifyControllerStateChangeForDisconnectScreen();

	/**
	 * This will check the state of the connected controllers to the player. If they do not have
	 * any controllers connected to them, then we should display the Disconnect menu. If they do have
	 * controllers connected to them, then we can hide the disconnect menu if its showing.
	 */
	virtual void ProcessControllerDevicesHavingChangedForDisconnectScreen();

	/**
     * Returns true if this platform supports a "controller disconnected" screen. 
     */
    virtual bool ShouldPlatformDisplayControllerDisconnectScreen() const;
	
	/**
	* Pushes the ControllerDisconnectedMenuClass to the Menu layer (UI.Layer.Menu)
	*/
	UFUNCTION(BlueprintNativeEvent, Category="Controller Disconnect Menu")
	void DisplayControllerDisconnectedMenu();

	/**
	* Hides the controller disconnected menu if it is active.
	*/
	UFUNCTION(BlueprintNativeEvent, Category="Controller Disconnect Menu")
	void HideControllerDisconnectedMenu();
	
	/** 
	* The widget which should be presented to the user if all of their controllers are disconnected.
	*/
	UPROPERTY(EditDefaultsOnly, Category="Controller Disconnect Menu")
	TSubclassOf<UBlueBattleControllerDisconnectedScreen> ControllerDisconnectedScreen;

	/**
	 * The platform tags that are required in order to show the "Controller Disconnected" screen.
	 *
	 * If these tags are not set in the INI file for this platform, then the controller disconnect screen
	 * will not ever be displayed. 
	 */
	UPROPERTY(EditDefaultsOnly, Category="Controller Disconnect Menu")
	FGameplayTagContainer PlatformRequiresControllerDisconnectScreen;

	/** Pointer to the active "Controller Disconnected" menu if there is one. */
	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> SpawnedControllerDisconnectScreen;
	/** Handle from the FSTicker for when we want to process the controller state of our player */
	FTSTicker::FDelegateHandle RequestProcessControllerStateHandle;
};
