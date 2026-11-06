// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModels/BlueBattleAttributeViewModel.h"

#include "CommonLocalPlayer.h"
#include "Combat/AttributeComponent.h"
#include "GameFramework/Pawn.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleAttributeViewModel)

void UBlueBattleAttributeViewModel::InitializeForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	Deinitialize();

	if (UCommonLocalPlayer* CommonLocalPlayer = Cast<UCommonLocalPlayer>(LocalPlayer))
	{
		TrackedLocalPlayer = CommonLocalPlayer;
		PawnSetHandle = CommonLocalPlayer->CallAndRegister_OnPlayerPawnSet(
			UCommonLocalPlayer::FPlayerPawnSetDelegate::FDelegate::CreateUObject(this, &ThisClass::HandlePawnSet));
	}
}

void UBlueBattleAttributeViewModel::Deinitialize()
{
	if (UCommonLocalPlayer* LocalPlayer = TrackedLocalPlayer.Get())
	{
		LocalPlayer->OnPlayerPawnSet.Remove(PawnSetHandle);
	}
	TrackedLocalPlayer.Reset();
	PawnSetHandle.Reset();

	SetAttributeComponent(nullptr);
}

void UBlueBattleAttributeViewModel::HandlePawnSet(UCommonLocalPlayer* LocalPlayer, APawn* Pawn)
{
	SetFromActor(Pawn);
}

void UBlueBattleAttributeViewModel::SetFromActor(AActor* Actor)
{
	SetAttributeComponent(Actor ? Actor->FindComponentByClass<UAttributeComponent>() : nullptr);
}

void UBlueBattleAttributeViewModel::SetAttributeComponent(UAttributeComponent* InAttributeComponent)
{
	if (AttributeComponent.Get() == InAttributeComponent)
	{
		return;
	}

	if (UAttributeComponent* OldComponent = AttributeComponent.Get())
	{
		OldComponent->OnHPChanged.RemoveDynamic(this, &ThisClass::HandleHPChanged);
		OldComponent->OnMPChanged.RemoveDynamic(this, &ThisClass::HandleMPChanged);
		OldComponent->OnEnergyValueChanged.RemoveDynamic(this, &ThisClass::HandleEnergyChanged);
		OldComponent->OnDeath.RemoveDynamic(this, &ThisClass::HandleDeath);
	}

	AttributeComponent = InAttributeComponent;

	if (InAttributeComponent)
	{
		InAttributeComponent->OnHPChanged.AddDynamic(this, &ThisClass::HandleHPChanged);
		InAttributeComponent->OnMPChanged.AddDynamic(this, &ThisClass::HandleMPChanged);
		InAttributeComponent->OnEnergyValueChanged.AddDynamic(this, &ThisClass::HandleEnergyChanged);
		InAttributeComponent->OnDeath.AddDynamic(this, &ThisClass::HandleDeath);
	}

	PullFromComponent();
}

void UBlueBattleAttributeViewModel::PullFromComponent()
{
	const UAttributeComponent* Component = AttributeComponent.Get();
	SetHPValues(Component ? Component->HP : 0.f, Component ? Component->MaxHP : 0.f);
	SetMPValues(Component ? Component->MP : 0.f, Component ? Component->MaxMP : 0.f);
	SetEnergyValues(Component ? Component->EnergyValue : 0.f, Component ? Component->MaxEnergyValue : 0.f);
	UE_MVVM_SET_PROPERTY_VALUE(bCanEX, Component ? Component->CanEX : false);
	UE_MVVM_SET_PROPERTY_VALUE(bIsDead, Component ? Component->IsDead : false);
}

void UBlueBattleAttributeViewModel::HandleHPChanged(float NewHP, float NewMaxHP, float Percent)
{
	SetHPValues(NewHP, NewMaxHP);
}

void UBlueBattleAttributeViewModel::HandleMPChanged(float NewMP, float NewMaxMP, float Percent)
{
	SetMPValues(NewMP, NewMaxMP);
}

void UBlueBattleAttributeViewModel::HandleEnergyChanged(float NewEnergy, float NewMaxEnergy, float Percent)
{
	SetEnergyValues(NewEnergy, NewMaxEnergy);
	if (const UAttributeComponent* Component = AttributeComponent.Get())
	{
		UE_MVVM_SET_PROPERTY_VALUE(bCanEX, Component->CanEX);
	}
}

void UBlueBattleAttributeViewModel::HandleDeath(AActor* Killer)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsDead, true);
}

void UBlueBattleAttributeViewModel::SetHPValues(float NewHP, float NewMaxHP)
{
	const bool bHPChanged = UE_MVVM_SET_PROPERTY_VALUE(HP, NewHP);
	const bool bMaxChanged = UE_MVVM_SET_PROPERTY_VALUE(MaxHP, NewMaxHP);
	if (bHPChanged || bMaxChanged)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHPPercent);
	}
}

void UBlueBattleAttributeViewModel::SetMPValues(float NewMP, float NewMaxMP)
{
	const bool bMPChanged = UE_MVVM_SET_PROPERTY_VALUE(MP, NewMP);
	const bool bMaxChanged = UE_MVVM_SET_PROPERTY_VALUE(MaxMP, NewMaxMP);
	if (bMPChanged || bMaxChanged)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetMPPercent);
	}
}

void UBlueBattleAttributeViewModel::SetEnergyValues(float NewEnergy, float NewMaxEnergy)
{
	const bool bEnergyChanged = UE_MVVM_SET_PROPERTY_VALUE(Energy, NewEnergy);
	const bool bMaxChanged = UE_MVVM_SET_PROPERTY_VALUE(MaxEnergy, NewMaxEnergy);
	if (bEnergyChanged || bMaxChanged)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetEnergyPercent);
	}
}
