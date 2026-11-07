// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/ComboComponent.h"
#include "Animation/AnimMontage.h"
#include "BlueBattleLogChannels.h"
#include "Combat/AttributeComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Mass/BlueBattleMassFragments.h"
#include "Mass/BlueBattleUnitSubsystem.h"

UComboComponent::UComboComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

UBlueBattleUnitSubsystem* UComboComponent::GetUnitSubsystemForWrite(const TCHAR* Operation) const
{
	UBlueBattleUnitSubsystem* UnitSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UBlueBattleUnitSubsystem>() : nullptr;
	if (!MassEntity.IsSet() || !UnitSubsystem)
	{
		UE_LOG(LogBlueBattle, Warning, TEXT("%s: %s ignored, Mass entity not bound yet"), *GetNameSafe(GetOwner()), Operation);
		return nullptr;
	}
	return UnitSubsystem;
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
	UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("PerformAttack"));
	int32 ComboStep = 0;
	bool bEX = false;
	UAnimMontage* Skill = nullptr;
	if (UnitSubsystem && UnitSubsystem->TryStartAttack(MassEntity, AttackType, ComboStep, bEX, Skill))
	{
		OnPlayAttackMontage(AttackType, bEX, ComboStep, Skill);
	}
}

float UComboComponent::ReportHit(AActor* Target)
{
	const UAttributeComponent* TargetAttributes = Target ? Target->FindComponentByClass<UAttributeComponent>() : nullptr;
	if (!TargetAttributes || Target == GetOwner())
	{
		return 0.f;
	}

	UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("ReportHit"));
	return UnitSubsystem ? UnitSubsystem->ResolveHit(MassEntity, TargetAttributes->GetMassEntity(), GetOwner()) : 0.f;
}

bool UComboComponent::PerformDodge()
{
	UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("PerformDodge"));
	if (UnitSubsystem && UnitSubsystem->TryStartDodge(MassEntity))
	{
		OnPlayDodgeMontage();
		return true;
	}
	return false;
}

void UComboComponent::ActivateBurst()
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("ActivateBurst")))
	{
		UnitSubsystem->TryActivateBurst(MassEntity);
	}
}

void UComboComponent::EndAttack()
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("EndAttack")))
	{
		UnitSubsystem->EndAttack(MassEntity);
	}
}

void UComboComponent::EndDodge()
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("EndDodge")))
	{
		UnitSubsystem->EndDodge(MassEntity);
	}
}

void UComboComponent::ResetCombo()
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("ResetCombo")))
	{
		UnitSubsystem->ResetCombo(MassEntity);
	}
}

void UComboComponent::OpenComboWindow()
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("OpenComboWindow")))
	{
		UnitSubsystem->SetComboWindowOpen(MassEntity, true);
	}
}

void UComboComponent::CloseComboWindow()
{
	if (UBlueBattleUnitSubsystem* UnitSubsystem = GetUnitSubsystemForWrite(TEXT("CloseComboWindow")))
	{
		UnitSubsystem->SetComboWindowOpen(MassEntity, false);
	}
}

void UComboComponent::ExportToMass(FBlueBattleComboParamsFragment& OutParams, FBlueBattleSkillSetFragment& OutSkillSet) const
{
	OutParams.BurstDuration = BurstDuration;
	OutParams.BurstEnergyCost = BurstEnergyCost;
	OutParams.DodgeCooldown = DodgeCooldown;

	OutSkillSet.LightAttacks = LightAttackSkills;
	OutSkillSet.HeavyAttacks = HeavyAttackSkills;
	OutSkillSet.EXLightAttacks = EXLightAttackSkills;
	OutSkillSet.EXHeavyAttacks = EXHeavyAttackSkills;
	OutSkillSet.SkillDamage = SkillDamageTable;
}

void UComboComponent::ApplyMassSnapshot(const FBlueBattleComboFragment& Combo, bool bBursting)
{
	const int32 PreviousComboCount = ComboCount;
	const bool bWasBursting = bExplosion;

	ComboCount = Combo.ComboIndex;
	bIsAttacking = Combo.bAttacking;
	bEvasion = Combo.bEvasion;
	bComboWindowOpen = Combo.bComboWindowOpen;
	bExplosion = bBursting;

	if (ComboCount != PreviousComboCount)
	{
		if (ComboCount == 0)
		{
			OnComboReset.Broadcast();
		}
		else
		{
			OnComboStepChanged.Broadcast(ComboCount, Combo.LastAttackType);
		}
	}

	if (bBursting != bWasBursting)
	{
		ApplyMoveSpeed(bBursting);
		SetComponentTickEnabled(bBursting);
		OnBurstStateChanged.Broadcast(bBursting);
	}
}

void UComboComponent::ApplyMoveSpeed(bool bBursting) const
{
	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = bBursting ? BurstMoveSpeed : NormalMoveSpeed;
		}
	}
}
