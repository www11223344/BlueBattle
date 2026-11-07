// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/AttributeComponent.h"
#include "BlueBattleLogChannels.h"
#include "Engine/World.h"
#include "Mass/BlueBattleMassFragments.h"
#include "Mass/BlueBattleUnitSubsystem.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// 实体创建前的初始显示：三条资源拉满（实体创建时以此为初值导出）
	HP = MaxHP;
	MP = MaxMP;
	EnergyValue = MaxEnergyValue;
	IsDead = false;
	bDeathBroadcast = false;
	CanEX = MaxEnergyValue > 0.f && EnergyValue >= MaxEnergyValue * 0.5f;

	BroadcastHP();
	BroadcastMP();
	BroadcastEnergyValue();
}

void UAttributeComponent::ExportToMass(FBlueBattleAttributeFragment& OutAttributes, FBlueBattleAttributeRegenFragment& OutRegen) const
{
	OutAttributes.HP = HP;
	OutAttributes.MaxHP = MaxHP;
	OutAttributes.MP = MP;
	OutAttributes.MaxMP = MaxMP;
	OutAttributes.Energy = EnergyValue;
	OutAttributes.MaxEnergy = MaxEnergyValue;
	OutAttributes.AttackPower = AttackPower;
	OutAttributes.Defense = Defense;

	OutRegen.MPRegenSpeed = MPRegenSpeed;
	OutRegen.EnergyDecaySpeed = EnergyValueDecaySpeed;
}

void UAttributeComponent::ApplyMassSnapshot(const FBlueBattleAttributeFragment& Attributes, bool bDead)
{
	const double PreviousHP = HP;

	HP = Attributes.HP;
	MaxHP = Attributes.MaxHP;
	MP = Attributes.MP;
	MaxMP = Attributes.MaxMP;
	EnergyValue = Attributes.Energy;
	MaxEnergyValue = Attributes.MaxEnergy;
	AttackPower = Attributes.AttackPower;
	Defense = Attributes.Defense;
	CanEX = Attributes.CanEX();
	IsDead = bDead;

	if (!FMath::IsNearlyEqual(LastBroadcastHP, HP) || !FMath::IsNearlyEqual(LastBroadcastMaxHP, MaxHP))
	{
		BroadcastHP();
	}
	if (!FMath::IsNearlyEqual(LastBroadcastMP, MP) || !FMath::IsNearlyEqual(LastBroadcastMaxMP, MaxMP))
	{
		BroadcastMP();
	}
	if (!FMath::IsNearlyEqual(LastBroadcastEnergyValue, EnergyValue) || !FMath::IsNearlyEqual(LastBroadcastMaxEnergyValue, MaxEnergyValue))
	{
		BroadcastEnergyValue();
	}
	if (HP < PreviousHP)
	{
		OnDamagedFeedback(PreviousHP - HP, Attributes.LastDamageCauser.Get());
	}
	if (bDead && !bDeathBroadcast)
	{
		bDeathBroadcast = true;
		AActor* Killer = Attributes.LastDamageCauser.Get();
		OnDeathFeedback(Killer);
		OnDeath.Broadcast(Killer);
	}
}

void UAttributeComponent::BroadcastHP()
{
	LastBroadcastHP = HP;
	LastBroadcastMaxHP = MaxHP;
	OnHPChanged.Broadcast(HP, MaxHP, GetHPPercent());
}

void UAttributeComponent::BroadcastMP()
{
	LastBroadcastMP = MP;
	LastBroadcastMaxMP = MaxMP;
	OnMPChanged.Broadcast(MP, MaxMP, MaxMP > 0.f ? MP / MaxMP : 0.f);
}

void UAttributeComponent::BroadcastEnergyValue()
{
	LastBroadcastEnergyValue = EnergyValue;
	LastBroadcastMaxEnergyValue = MaxEnergyValue;
	OnEnergyValueChanged.Broadcast(EnergyValue, MaxEnergyValue, MaxEnergyValue > 0.f ? EnergyValue / MaxEnergyValue : 0.f);
}

float UAttributeComponent::GetHPPercent() const
{
	return MaxHP > 0.f ? HP / MaxHP : 0.f;
}

UBlueBattleUnitSubsystem* UAttributeComponent::GetUnitSubsystemForWrite(const TCHAR* Operation) const
{
	UBlueBattleUnitSubsystem* UnitSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UBlueBattleUnitSubsystem>() : nullptr;
	if (!MassEntity.IsSet() || !UnitSubsystem)
	{
		UE_LOG(LogBlueBattle, Warning, TEXT("%s: %s ignored, Mass entity not bound yet"), *GetNameSafe(GetOwner()), Operation);
		return nullptr;
	}
	return UnitSubsystem;
}

float UAttributeComponent::ApplyDamage(float RawDamage, AActor* DamageCauser)
{
	UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("ApplyDamage"));
	return UnitSubsystem ? UnitSubsystem->ApplyDamage(MassEntity, RawDamage, DamageCauser) : 0.f;
}

void UAttributeComponent::Heal(float Amount)
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("Heal")))
	{
		UnitSubsystem->Heal(MassEntity, Amount);
	}
}

void UAttributeComponent::ConsumeMP(float Cost)
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("ConsumeMP")))
	{
		UnitSubsystem->ConsumeMP(MassEntity, Cost);
	}
}

bool UAttributeComponent::ConsumeEnergyValue(float Cost)
{
	UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("ConsumeEnergyValue"));
	return UnitSubsystem && UnitSubsystem->ConsumeEnergy(MassEntity, Cost);
}

void UAttributeComponent::AddEnergyValue(float Amount)
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("AddEnergyValue")))
	{
		UnitSubsystem->AddEnergy(MassEntity, Amount);
	}
}
