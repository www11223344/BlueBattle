// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueBattleHUDLayout.h"

#include "CommonUIExtensions.h"
#include "CommonUISettings.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "Input/CommonUIInputTypes.h"
#include "ICommonUIModule.h"
#include "BlueBattleLogChannels.h"
#include "Components/OverlaySlot.h"
#include "Engine/LocalPlayer.h"
#include "NativeGameplayTags.h"
#include "UI/Foundation/BlueBattleControllerDisconnectedScreen.h"
#include "UI/BlueBattleActivatableWidget.h"
#include "UI/Subsystem/BlueBattleUIManagerSubsystem.h"

#if WITH_EDITOR
#include "CommonUIVisibilitySubsystem.h"
#endif	// WITH_EDITOR

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleHUDLayout)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Menu");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_ACTION_ESCAPE, "UI.Action.Escape");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_PrimarlyController, "Platform.Trait.Input.PrimarlyController");

UBlueBattleHUDLayout::UBlueBattleHUDLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SpawnedControllerDisconnectScreen(nullptr)
{
	// By default, only primarily controller platforms require a disconnect screen. 
	PlatformRequiresControllerDisconnectScreen.AddTag(TAG_Platform_Trait_Input_PrimarlyController);
}

void UBlueBattleHUDLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	FBindUIActionArgs EscapeActionArgs(FUIActionTag::ConvertChecked(TAG_UI_ACTION_ESCAPE), false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction));
	// 绑定默认只在 Menu 模式响应，而 HUD 布局激活时的输入模式是 Game（All 也不匹配 Game）
	EscapeActionArgs.InputMode = ECommonInputMode::Game;
	RegisterUIActionBinding(EscapeActionArgs);
	// If we can display a controller disconnect screen, then listen for the controller state change delegates
	if (ShouldPlatformDisplayControllerDisconnectScreen())
	{
		// Bind to when input device connections change
		IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
		DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::HandleInputDeviceConnectionChanged);
		DeviceMapper.GetOnInputDevicePairingChange().AddUObject(this, &ThisClass::HandleInputDevicePairingChanged);	
	}
}

void UBlueBattleHUDLayout::NativeDestruct()
{
	Super::NativeDestruct();

	// Remove bindings to input device connection changing
	IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().RemoveAll(this);
	DeviceMapper.GetOnInputDevicePairingChange().RemoveAll(this);

	if (RequestProcessControllerStateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RequestProcessControllerStateHandle);
		RequestProcessControllerStateHandle.Reset();
	}
}

void UBlueBattleHUDLayout::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdatePlayerScreenRegion(MyGeometry);
}

void UBlueBattleHUDLayout::UpdatePlayerScreenRegion(const FGeometry& MyGeometry)
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UOverlaySlot* RegionSlot = PlayerScreenRegion ? Cast<UOverlaySlot>(PlayerScreenRegion->Slot) : nullptr;
	if (!LocalPlayer || !RegionSlot)
	{
		return;
	}

	const FVector2D LayoutSize = MyGeometry.GetLocalSize();
	const FVector2D Origin = LocalPlayer->Origin;
	const FVector2D Size = LocalPlayer->Size;
	const FMargin Padding(
		static_cast<float>(Origin.X * LayoutSize.X),
		static_cast<float>(Origin.Y * LayoutSize.Y),
		static_cast<float>(FMath::Max(0.0, 1.0 - Origin.X - Size.X) * LayoutSize.X),
		static_cast<float>(FMath::Max(0.0, 1.0 - Origin.Y - Size.Y) * LayoutSize.Y));

	if (!(RegionSlot->GetPadding() == Padding))
	{
		RegionSlot->SetPadding(Padding);
	}
}

void UBlueBattleHUDLayout::HandleEscapeAction()
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (UBlueBattleUIManagerSubsystem* UIManager = LocalPlayer ? LocalPlayer->GetGameInstance()->GetSubsystem<UBlueBattleUIManagerSubsystem>() : nullptr)
	{
		UIManager->OpenPauseMenu(LocalPlayer);
	}
}

void UBlueBattleHUDLayout::HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId)
{
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();

	ensure(OwningLocalPlayerId.IsValid());

	// This device connection change happened to a different player, ignore it for us.
	if (PlatformUserId != OwningLocalPlayerId)
	{
		return;
	}

	NotifyControllerStateChangeForDisconnectScreen();
}

void UBlueBattleHUDLayout::HandleInputDevicePairingChanged(FInputDeviceId InputDeviceId, FPlatformUserId NewUserPlatformId, FPlatformUserId OldUserPlatformId)
{
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();

	ensure(OwningLocalPlayerId.IsValid());

	// If this pairing change was related to our local player, notify of a change.
	if (NewUserPlatformId == OwningLocalPlayerId || OldUserPlatformId == OwningLocalPlayerId)
	{
		NotifyControllerStateChangeForDisconnectScreen();	
	}
}

bool UBlueBattleHUDLayout::ShouldPlatformDisplayControllerDisconnectScreen() const
{
	// We only want this menu on primarily controller platforms
	bool bHasAllRequiredTags = ICommonUIModule::GetSettings().GetPlatformTraits().HasAll(PlatformRequiresControllerDisconnectScreen);

	// Check the tags that we may be emulating in the editor too
#if WITH_EDITOR
	const FGameplayTagContainer& PlatformEmulationTags = UCommonUIVisibilitySubsystem::Get(GetOwningLocalPlayer())->GetVisibilityTags();
	bHasAllRequiredTags |= PlatformEmulationTags.HasAll(PlatformRequiresControllerDisconnectScreen);
#endif	// WITH_EDITOR

	return bHasAllRequiredTags;
}

void UBlueBattleHUDLayout::NotifyControllerStateChangeForDisconnectScreen()
{
	// We should only ever get here if we have bound to the controller state change delegates
	ensure(ShouldPlatformDisplayControllerDisconnectScreen());

	// If we haven't already, queue the processing of device state for next tick.
	if (!RequestProcessControllerStateHandle.IsValid())
	{
		RequestProcessControllerStateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float DeltaTime)
		{
			RequestProcessControllerStateHandle.Reset();
			ProcessControllerDevicesHavingChangedForDisconnectScreen();
			return false;
		}));
	}
}

void UBlueBattleHUDLayout::ProcessControllerDevicesHavingChangedForDisconnectScreen()
{
	// We should only ever get here if we have bound to the controller state change delegates
	ensure(ShouldPlatformDisplayControllerDisconnectScreen());
	
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();
	
	ensure(OwningLocalPlayerId.IsValid());

	// Get all input devices mapped to our player
	const IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
    TArray<FInputDeviceId> MappedInputDevices;
    const int32 NumDevicesMappedToUser = DeviceMapper.GetAllInputDevicesForUser(OwningLocalPlayerId, OUT MappedInputDevices);

    // Check if there are any other connected GAMEPAD devices mapped to this platform user. 
    bool bHasConnectedController = false;

    for (const FInputDeviceId MappedDevice : MappedInputDevices)
    {
    	if (DeviceMapper.GetInputDeviceConnectionState(MappedDevice) == EInputDeviceConnectionState::Connected)
    	{
    		const FHardwareDeviceIdentifier HardwareInfo = UInputDeviceSubsystem::Get()->GetInputDeviceHardwareIdentifier(MappedDevice);
    		if (HardwareInfo.PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad)
    		{
    			bHasConnectedController = true;
    		}
    	}			
    }

    // If there are no gamepad input devices mapped to this user, then we want to pop the toast saying to re-connect them
    if (!bHasConnectedController)
    {
    	DisplayControllerDisconnectedMenu();
    }
	// Otherwise we can hide the screen if it is currently being shown
	else if (SpawnedControllerDisconnectScreen)
	{
		HideControllerDisconnectedMenu();
	}
}

void UBlueBattleHUDLayout::DisplayControllerDisconnectedMenu_Implementation()
{
	UE_LOG(LogBlueBattle, Log, TEXT("[%hs] Display controller disconnected menu!"), __func__);

	if (ControllerDisconnectedScreen)
	{
		// Push the "controller disconnected" widget to the menu layer
		SpawnedControllerDisconnectScreen = UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, ControllerDisconnectedScreen);
	}
}

void UBlueBattleHUDLayout::HideControllerDisconnectedMenu_Implementation()
{
	UE_LOG(LogBlueBattle, Log, TEXT("[%hs] Hide controller disconnected menu!"), __func__);
	
	UCommonUIExtensions::PopContentFromLayer(SpawnedControllerDisconnectScreen);
	SpawnedControllerDisconnectScreen = nullptr;
}