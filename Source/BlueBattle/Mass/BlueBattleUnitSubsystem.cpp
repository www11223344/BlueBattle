// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/BlueBattleUnitSubsystem.h"
#include "Animation/AnimMontage.h"
#include "BlueBattleLogChannels.h"
#include "Engine/World.h"
#include "Mass/BlueBattleMassFragments.h"
#include "MassCommandBuffer.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"

FMassEntityManager* UBlueBattleUnitSubsystem::GetEntityManager() const
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	return EntitySubsystem ? &EntitySubsystem->GetMutableEntityManager() : nullptr;
}

FBlueBattleAttributeFragment* UBlueBattleUnitSubsystem::GetMutableAttributes(FMassEntityHandle Entity) const
{
	FMassEntityManager* EntityManager = GetEntityManager();
	if (!EntityManager || !EntityManager->IsEntityActive(Entity))
	{
		return nullptr;
	}
	return EntityManager->GetFragmentDataPtr<FBlueBattleAttributeFragment>(Entity);
}

const FBlueBattleAttributeFragment* UBlueBattleUnitSubsystem::GetAttributes(FMassEntityHandle Entity) const
{
	return GetMutableAttributes(Entity);
}

bool UBlueBattleUnitSubsystem::IsDead(FMassEntityHandle Entity) const
{
	const FBlueBattleAttributeFragment* Attributes = GetAttributes(Entity);
	if (!Attributes)
	{
		return false;
	}
	return Attributes->HP <= 0.f || GetEntityManager()->DoesEntityHaveElement<FBlueBattleDeadTag>(Entity);
}

namespace
{
	/** Mass 处理阶段内（如 Translator 广播的委托里回调到本子系统）结构变更必须延迟执行 */
	template <typename TTag>
	void AddTagSafe(FMassEntityManager& EntityManager, FMassEntityHandle Entity)
	{
		if (EntityManager.IsProcessing())
		{
			EntityManager.Defer().AddTag<TTag>(Entity);
		}
		else
		{
			EntityManager.AddTagToEntity(Entity, TTag::StaticStruct());
		}
	}
}

void UBlueBattleUnitSubsystem::MarkDead(FMassEntityHandle Entity)
{
	AddTagSafe<FBlueBattleDeadTag>(*GetEntityManager(), Entity);
}

FBlueBattleComboFragment* UBlueBattleUnitSubsystem::GetMutableCombo(FMassEntityHandle Entity) const
{
	FMassEntityManager* EntityManager = GetEntityManager();
	if (!EntityManager || !EntityManager->IsEntityActive(Entity))
	{
		return nullptr;
	}
	return EntityManager->GetFragmentDataPtr<FBlueBattleComboFragment>(Entity);
}

bool UBlueBattleUnitSubsystem::TryStartAttack(FMassEntityHandle Entity, EAttackType AttackType, int32& OutComboStep, bool& bOutEX, UAnimMontage*& OutSkill)
{
	FBlueBattleComboFragment* Combo = GetMutableCombo(Entity);
	if (!Combo || IsDead(Entity) || Combo->bAttacking || Combo->bEvasion)
	{
		return false;
	}

	const FBlueBattleSkillSetFragment* SkillSet = GetEntityManager()->GetFragmentDataPtr<FBlueBattleSkillSetFragment>(Entity);
	const bool bEX = IsBursting(Entity);
	const TArray<TObjectPtr<UAnimMontage>>* Attacks = SkillSet ? &SkillSet->GetAttacks(AttackType, bEX) : nullptr;
	if (!Attacks || Attacks->IsEmpty())
	{
		UE_LOG(LogBlueBattle, Warning, TEXT("TryStartAttack: no skills configured for attack type %d (EX=%d)"), static_cast<int32>(AttackType), bEX);
		return false;
	}

	const int32 ComboStep = Combo->ComboIndex++;
	UAnimMontage* Skill = (*Attacks)[ComboStep % Attacks->Num()];

	Combo->bEvasion = false;
	Combo->bAttacking = true;
	Combo->LastAttackType = AttackType;
	Combo->CurrentSkill = Skill;
	Combo->CurrentSkillDamage = SkillSet->SkillDamage.FindRef(Skill);

	OutComboStep = ComboStep;
	bOutEX = bEX;
	OutSkill = Skill;
	return true;
}

float UBlueBattleUnitSubsystem::ResolveHit(FMassEntityHandle Attacker, FMassEntityHandle Target, AActor* DamageCauser)
{
	const FBlueBattleComboFragment* Combo = GetMutableCombo(Attacker);
	if (!Combo || !Combo->bAttacking || Combo->CurrentSkillDamage <= 0.f || Attacker == Target || IsDead(Attacker) || IsDead(Target))
	{
		return 0.f;
	}

	const float SkillDamage = Combo->CurrentSkillDamage;
	const float Dealt = ApplyDamage(Target, SkillDamage, DamageCauser);
	if (Dealt > 0.f)
	{
		AddEnergy(Attacker, SkillDamage);
	}
	return Dealt;
}

bool UBlueBattleUnitSubsystem::TryStartDodge(FMassEntityHandle Entity)
{
	FBlueBattleComboFragment* Combo = GetMutableCombo(Entity);
	if (!Combo || IsDead(Entity) || Combo->bEvasion || Combo->DodgeCooldownRemaining > 0.f)
	{
		return false;
	}

	const FBlueBattleComboParamsFragment* Params = GetEntityManager()->GetFragmentDataPtr<FBlueBattleComboParamsFragment>(Entity);
	Combo->bAttacking = false;
	Combo->bEvasion = true;
	Combo->DodgeCooldownRemaining = Params ? Params->DodgeCooldown : 0.f;
	return true;
}

void UBlueBattleUnitSubsystem::EndAttack(FMassEntityHandle Entity)
{
	if (FBlueBattleComboFragment* Combo = GetMutableCombo(Entity))
	{
		Combo->bAttacking = false;
	}
}

void UBlueBattleUnitSubsystem::EndDodge(FMassEntityHandle Entity)
{
	if (FBlueBattleComboFragment* Combo = GetMutableCombo(Entity))
	{
		Combo->bEvasion = false;
		Combo->bAttacking = false;
	}
}

void UBlueBattleUnitSubsystem::ResetCombo(FMassEntityHandle Entity)
{
	if (FBlueBattleComboFragment* Combo = GetMutableCombo(Entity))
	{
		Combo->ComboIndex = 0;
		Combo->bAttacking = false;
		Combo->bEvasion = false;
		Combo->bComboWindowOpen = false;
	}
}

void UBlueBattleUnitSubsystem::SetComboWindowOpen(FMassEntityHandle Entity, bool bOpen)
{
	if (FBlueBattleComboFragment* Combo = GetMutableCombo(Entity))
	{
		Combo->bComboWindowOpen = bOpen;
	}
}

bool UBlueBattleUnitSubsystem::TryActivateBurst(FMassEntityHandle Entity)
{
	FMassEntityManager* EntityManager = GetEntityManager();
	FBlueBattleComboFragment* Combo = GetMutableCombo(Entity);
	const FBlueBattleAttributeFragment* Attributes = GetAttributes(Entity);
	const FBlueBattleComboParamsFragment* Params = Combo ? EntityManager->GetFragmentDataPtr<FBlueBattleComboParamsFragment>(Entity) : nullptr;
	if (!Combo || !Attributes || !Params || IsDead(Entity) || IsBursting(Entity) || !Attributes->CanEX())
	{
		return false;
	}
	if (!ConsumeEnergy(Entity, Params->BurstEnergyCost))
	{
		return false;
	}

	Combo->BurstRemaining = Params->BurstDuration;
	AddTagSafe<FBlueBattleBurstTag>(*EntityManager, Entity);
	return true;
}

bool UBlueBattleUnitSubsystem::IsBursting(FMassEntityHandle Entity) const
{
	FMassEntityManager* EntityManager = GetEntityManager();
	return EntityManager && EntityManager->IsEntityActive(Entity) && EntityManager->DoesEntityHaveElement<FBlueBattleBurstTag>(Entity);
}

float UBlueBattleUnitSubsystem::ApplyDamage(FMassEntityHandle Entity, float RawDamage, AActor* DamageCauser)
{
	FBlueBattleAttributeFragment* Attributes = GetMutableAttributes(Entity);
	if (!Attributes || RawDamage <= 0.f || IsDead(Entity))
	{
		return 0.f;
	}

	const float FinalDamage = FMath::Max(0.f, RawDamage - Attributes->Defense);
	const float OldHP = Attributes->HP;
	Attributes->HP = FMath::Clamp(Attributes->HP - FinalDamage, 0.f, Attributes->MaxHP);
	Attributes->LastDamageCauser = DamageCauser;

	if (Attributes->HP <= 0.f)
	{
		MarkDead(Entity);
	}
	return OldHP - Attributes->HP;
}

void UBlueBattleUnitSubsystem::Heal(FMassEntityHandle Entity, float Amount)
{
	FBlueBattleAttributeFragment* Attributes = GetMutableAttributes(Entity);
	if (!Attributes || Amount <= 0.f || IsDead(Entity))
	{
		return;
	}
	Attributes->HP = FMath::Clamp(Attributes->HP + Amount, 0.f, Attributes->MaxHP);
}

void UBlueBattleUnitSubsystem::ConsumeMP(FMassEntityHandle Entity, float Cost)
{
	FBlueBattleAttributeFragment* Attributes = GetMutableAttributes(Entity);
	if (!Attributes || Cost <= 0.f)
	{
		return;
	}
	Attributes->MP = FMath::Clamp(Attributes->MP - Cost, 0.f, Attributes->MaxMP);
}

bool UBlueBattleUnitSubsystem::ConsumeEnergy(FMassEntityHandle Entity, float Cost)
{
	FBlueBattleAttributeFragment* Attributes = GetMutableAttributes(Entity);
	if (!Attributes)
	{
		return false;
	}
	if (Cost <= 0.f)
	{
		return true;
	}
	if (Attributes->Energy < Cost)
	{
		return false;
	}
	Attributes->Energy = FMath::Clamp(Attributes->Energy - Cost, 0.f, Attributes->MaxEnergy);
	return true;
}

void UBlueBattleUnitSubsystem::AddEnergy(FMassEntityHandle Entity, float Amount)
{
	FBlueBattleAttributeFragment* Attributes = GetMutableAttributes(Entity);
	if (!Attributes || Amount <= 0.f)
	{
		return;
	}
	Attributes->Energy = FMath::Clamp(Attributes->Energy + Amount, 0.f, Attributes->MaxEnergy);
}
