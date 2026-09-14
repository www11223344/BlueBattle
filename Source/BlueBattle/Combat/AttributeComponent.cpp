// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/AttributeComponent.h"

UAttributeComponent::UAttributeComponent()
{
	// 需要 Tick 来做魔法回复 / 能量衰减 / 死亡判定
	PrimaryComponentTick.bCanEverTick = true;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化：三条资源拉满
	HP = MaxHP;
	MP = MaxMP;
	EnergyValue = MaxEnergyValue;
	IsDead = false;
	CanEX = MaxEnergyValue > 0.f && EnergyValue >= MaxEnergyValue * 0.5f;

	BroadcastAll();
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsDead)
	{
		return;
	}

	const float OldMP = MP;
	const float OldEnergyValue = EnergyValue;

	// 魔法值向上限匀速回复
	MP = FMath::FInterpConstantTo(MP, MaxMP, DeltaTime, MPRegenSpeed);

	// 能量值向 0 匀速衰减
	EnergyValue = FMath::FInterpConstantTo(EnergyValue, 0.f, DeltaTime, EnergyValueDecaySpeed);

	// 能量达到一半即可释放爆气/EX
	CanEX = MaxEnergyValue > 0.f && EnergyValue >= MaxEnergyValue * 0.5f;

	if (!FMath::IsNearlyEqual(OldMP, MP))
	{
		OnMPChanged.Broadcast(MP, MaxMP, MaxMP > 0.f ? MP / MaxMP : 0.f);
	}
	if (!FMath::IsNearlyEqual(OldEnergyValue, EnergyValue))
	{
		OnEnergyValueChanged.Broadcast(EnergyValue, MaxEnergyValue, MaxEnergyValue > 0.f ? EnergyValue / MaxEnergyValue : 0.f);
	}

	// 死亡判定：HP<=0
	if (HP <= 0.f && !IsDead)
	{
		IsDead = true;
		OnDeath.Broadcast(nullptr);
	}
}

void UAttributeComponent::BroadcastAll()
{
	OnHPChanged.Broadcast(HP, MaxHP, GetHPPercent());
	OnMPChanged.Broadcast(MP, MaxMP, MaxMP > 0.f ? MP / MaxMP : 0.f);
	OnEnergyValueChanged.Broadcast(EnergyValue, MaxEnergyValue, MaxEnergyValue > 0.f ? EnergyValue / MaxEnergyValue : 0.f);
}

float UAttributeComponent::GetHPPercent() const
{
	return MaxHP > 0.f ? HP / MaxHP : 0.f;
}

float UAttributeComponent::ApplyDamage(float RawDamage, AActor* DamageCauser)
{
	if (IsDead || RawDamage <= 0.f)
	{
		return 0.f;
	}

	// 伤害公式：最终伤害 = RawDamage - 防御（Defense 默认 0）
	const float FinalDamage = FMath::Max(0.f, RawDamage - Defense);
	const float OldHP = HP;
	HP = FMath::Clamp(HP - FinalDamage, 0.f, MaxHP);

	OnHPChanged.Broadcast(HP, MaxHP, GetHPPercent());

	if (HP <= 0.f && !IsDead)
	{
		IsDead = true;
		OnDeath.Broadcast(DamageCauser);
	}

	return OldHP - HP;
}

void UAttributeComponent::Heal(float Amount)
{
	if (IsDead || Amount <= 0.f)
	{
		return;
	}

	HP = FMath::Clamp(HP + Amount, 0.f, MaxHP);
	OnHPChanged.Broadcast(HP, MaxHP, GetHPPercent());
}

void UAttributeComponent::ConsumeMP(float Cost)
{
	if (Cost <= 0.f)
	{
		return;
	}

	MP = FMath::Clamp(MP - Cost, 0.f, MaxMP);
	OnMPChanged.Broadcast(MP, MaxMP, MaxMP > 0.f ? MP / MaxMP : 0.f);
}

bool UAttributeComponent::ConsumeEnergyValue(float Cost)
{
	if (Cost <= 0.f)
	{
		return true;
	}
	if (EnergyValue < Cost)
	{
		return false;
	}

	EnergyValue = FMath::Clamp(EnergyValue - Cost, 0.f, MaxEnergyValue);
	CanEX = MaxEnergyValue > 0.f && EnergyValue >= MaxEnergyValue * 0.5f;
	OnEnergyValueChanged.Broadcast(EnergyValue, MaxEnergyValue, MaxEnergyValue > 0.f ? EnergyValue / MaxEnergyValue : 0.f);
	return true;
}

void UAttributeComponent::AddEnergyValue(float Amount)
{
	if (Amount <= 0.f)
	{
		return;
	}

	EnergyValue = FMath::Clamp(EnergyValue + Amount, 0.f, MaxEnergyValue);
	CanEX = MaxEnergyValue > 0.f && EnergyValue >= MaxEnergyValue * 0.5f;
	OnEnergyValueChanged.Broadcast(EnergyValue, MaxEnergyValue, MaxEnergyValue > 0.f ? EnergyValue / MaxEnergyValue : 0.f);
}
