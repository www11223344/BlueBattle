// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModels/BlueBattleBurstViewModel.h"

#include "CommonLocalPlayer.h"
#include "Combat/AttributeComponent.h"
#include "Combat/ComboComponent.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleBurstViewModel)

#define LOCTEXT_NAMESPACE "BlueBattleBurst"

void UBlueBattleBurstViewModel::InitializeForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	Deinitialize();

	if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
	{
		TrackedLocalPlayer = CommonLocalPlayer;
		PawnSetHandle = CommonLocalPlayer->CallAndRegister_OnPlayerPawnSet(
			UCommonLocalPlayer::FPlayerPawnSetDelegate::FDelegate::CreateUObject(this, &ThisClass::HandlePawnSet));
	}
}

void UBlueBattleBurstViewModel::Deinitialize()
{
	if (UCommonLocalPlayer* LocalPlayer = TrackedLocalPlayer.Get())
	{
		LocalPlayer->OnPlayerPawnSet.Remove(PawnSetHandle);
	}
	TrackedLocalPlayer.Reset();
	PawnSetHandle.Reset();

	BindComponents(nullptr, nullptr);
}

void UBlueBattleBurstViewModel::HandlePawnSet(UCommonLocalPlayer* LocalPlayer, APawn* Pawn)
{
	SetFromActor(Pawn);
}

void UBlueBattleBurstViewModel::SetFromActor(AActor* Actor)
{
	BindComponents(Actor ? Actor->FindComponentByClass<UAttributeComponent>() : nullptr,
		Actor ? Actor->FindComponentByClass<UComboComponent>() : nullptr);
}

void UBlueBattleBurstViewModel::BindComponents(UAttributeComponent* InAttributeComponent, UComboComponent* InComboComponent)
{
	if (UAttributeComponent* OldAttributes = AttributeComponent.Get(); OldAttributes && OldAttributes != InAttributeComponent)
	{
		OldAttributes->OnEnergyValueChanged.RemoveDynamic(this, &ThisClass::HandleEnergyChanged);
		OldAttributes->OnDeath.RemoveDynamic(this, &ThisClass::HandleDeath);
	}
	if (UComboComponent* OldCombo = ComboComponent.Get(); OldCombo && OldCombo != InComboComponent)
	{
		OldCombo->OnBurstStateChanged.RemoveDynamic(this, &ThisClass::HandleBurstStateChanged);
	}

	if (InAttributeComponent && InAttributeComponent != AttributeComponent.Get())
	{
		InAttributeComponent->OnEnergyValueChanged.AddDynamic(this, &ThisClass::HandleEnergyChanged);
		InAttributeComponent->OnDeath.AddDynamic(this, &ThisClass::HandleDeath);
	}
	if (InComboComponent && InComboComponent != ComboComponent.Get())
	{
		InComboComponent->OnBurstStateChanged.AddDynamic(this, &ThisClass::HandleBurstStateChanged);
	}

	AttributeComponent = InAttributeComponent;
	ComboComponent = InComboComponent;
	Refresh();
}

void UBlueBattleBurstViewModel::HandleEnergyChanged(float NewEnergy, float NewMaxEnergy, float Percent)
{
	Refresh();
}

void UBlueBattleBurstViewModel::HandleDeath(AActor* Killer)
{
	Refresh();
}

void UBlueBattleBurstViewModel::HandleBurstStateChanged(bool bActive)
{
	Refresh();
}

void UBlueBattleBurstViewModel::Refresh()
{
	const UAttributeComponent* Attributes = AttributeComponent.Get();
	const UComboComponent* Combo = ComboComponent.Get();

	const bool bNewIsBursting = Combo && Combo->IsBursting();
	const bool bNewCanBurst = Attributes && Attributes->IsAlive() && Attributes->fCanEX() && !bNewIsBursting;

	const bool bBurstingChanged = UE_MVVM_SET_PROPERTY_VALUE(bIsBursting, bNewIsBursting);
	const bool bCanBurstChanged = UE_MVVM_SET_PROPERTY_VALUE(bCanBurst, bNewCanBurst);
	if (bBurstingChanged || bCanBurstChanged)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStatusText);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStatusColor);
	}
}

FText UBlueBattleBurstViewModel::GetStatusText() const
{
	if (bIsBursting)
	{
		return LOCTEXT("Bursting", "爆气中");
	}
	return bCanBurst ? LOCTEXT("Ready", "可以爆气") : LOCTEXT("Unavailable", "不可爆气");
}

FLinearColor UBlueBattleBurstViewModel::GetStatusColor() const
{
	if (bIsBursting)
	{
		return BurstingColor;
	}
	return bCanBurst ? ReadyColor : UnavailableColor;
}

#undef LOCTEXT_NAMESPACE
