// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/ComboComponent.h"
#include "Combat/AttributeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UComboComponent::UComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UComboComponent::BeginPlay()
{
	Super::BeginPlay();

	ComboCount = 0;
	bIsAttacking = false;
	bEvasion = false;
	bExplosion = false;
	bComboWindowOpen = false;
}

UAttributeComponent* UComboComponent::GetAttribute() const
{
	if (const AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UAttributeComponent>();
	}
	return nullptr;
}

void UComboComponent::PerformLightAttack()
{
	PerformAttack(EAttackType::Light);
}

void UComboComponent::PerformHeavyAttack()
{
	PerformAttack(EAttackType::Heavy);
}

void UComboComponent::PerformAttack(EAttackType AttackType)
{
	// 死亡时不允许攻击
	if (const UAttributeComponent* Attr = GetAttribute())
	{
		if (Attr->fIsDead())
		{
			return;
		}
	}

	// 进入攻击状态、清除闪避（对应蓝图 攻击 / EX 内联分支）
	bEvasion = false;
	bIsAttacking = true;

	// 交给表现层选择并播放对应蒙太奇（普通 or EX 由 bExplosion 决定）+ 音效 + 记录当前技能
	OnPlayAttackMontage(AttackType, bExplosion, ComboCount);

	// 计数递增（表现层用 ComboCount % 数组长度 取段）
	++ComboCount;

	OnComboStepChanged.Broadcast(ComboCount, AttackType);
}

void UComboComponent::PerformDodge()
{
	if (const UAttributeComponent* Attr = GetAttribute())
	{
		if (Attr->fIsDead())
		{
			return;
		}
	}

	// 闪避打断攻击（对应蓝图 闪避：SetAttacking=false）
	bIsAttacking = false;

	// 交给蓝图按朝向选择并播放闪避蒙太奇
	OnPlayDodgeMontage();

	bEvasion = true;
}

void UComboComponent::ActivateBurst()
{
	// 已在爆气中则忽略
	if (bExplosion)
	{
		return;
	}

	UAttributeComponent* Attr = GetAttribute();
	if (!Attr || !Attr->fCanEX())
	{
		return;
	}

	// 消耗能量
	Attr->ConsumeEnergyValue(BurstEnergyCost);

	// 进入爆气：提升移动速度（对应蓝图 SetMaxWalkSpeed 600）
	bExplosion = true;
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Move = OwnerChar->GetCharacterMovement())
		{
			Move->MaxWalkSpeed = BurstMoveSpeed;
		}
	}

	OnBurstStateChanged.Broadcast(true);

	// N 秒后自动结束（对应蓝图 Delay 15s）
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(BurstTimerHandle, this, &UComboComponent::EndBurst, BurstDuration, false);
	}
}

void UComboComponent::EndBurst()
{
	bExplosion = false;
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Move = OwnerChar->GetCharacterMovement())
		{
			Move->MaxWalkSpeed = NormalMoveSpeed;
		}
	}

	OnBurstStateChanged.Broadcast(false);
}

void UComboComponent::EndAttack()
{
	bIsAttacking = false;
}

void UComboComponent::ResetCombo()
{
	if (ComboCount == 0 && !bIsAttacking && !bEvasion)
	{
		return;
	}

	ComboCount = 0;
	bIsAttacking = false;
	bEvasion = false;
	bComboWindowOpen = false;
	OnComboReset.Broadcast();
}
