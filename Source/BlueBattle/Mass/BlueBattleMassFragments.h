// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Combat/BlueBattleTypes.h"
#include "MassCommonFragments.h"
#include "MassEntityTypes.h"
#include "BlueBattleMassFragments.generated.h"

class UAnimMontage;
class UAttributeComponent;
class UComboComponent;

/** 标记由 BlueBattle 单位（WarriorCharacter 等）驱动创建的实体，后续 Processor / Translator 以此筛选。 */
USTRUCT()
struct FBlueBattleUnitTag : public FMassTag
{
	GENERATED_BODY()
};

/** 单位属性（权威数据）：生命 / 魔法 / 能量与攻防。表现层 UAttributeComponent 只读镜像这些值。 */
USTRUCT()
struct FBlueBattleAttributeFragment : public FMassFragment
{
	GENERATED_BODY()

	float HP = 100.f;
	float MaxHP = 100.f;
	float MP = 100.f;
	float MaxMP = 100.f;
	float Energy = 100.f;
	float MaxEnergy = 100.f;
	float AttackPower = 5.f;
	float Defense = 0.f;

	/** 每次命中造成的固定伤害（调试用），覆盖招式表；小于 0 时按招式表 */
	float DamageOverride = -1.f;

	/** 最近一次造成伤害的 Actor，死亡时作为击杀者广播 */
	TWeakObjectPtr<AActor> LastDamageCauser;

	bool CanEX() const { return MaxEnergy > 0.f && Energy >= MaxEnergy * 0.5f; }
};

/** 属性的持续变化速率：MP 向上限回复、能量向 0 衰减（每秒）。 */
USTRUCT()
struct FBlueBattleAttributeRegenFragment : public FMassFragment
{
	GENERATED_BODY()

	float MPRegenSpeed = 10.f;
	float EnergyDecaySpeed = 5.f;
};

/** 已死亡：停止回复，Translator 广播一次 OnDeath。 */
USTRUCT()
struct FBlueBattleDeadTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FBlueBattleAttributeComponentWrapperFragment : public FObjectWrapperFragment
{
	GENERATED_BODY()

	TWeakObjectPtr<UAttributeComponent> Component;
};

/** UBlueBattleAttributeToActorTranslator 的筛选标签 */
USTRUCT()
struct FBlueBattleAttributeCopyToActorTag : public FMassTag
{
	GENERATED_BODY()
};

/** 连招状态机（权威数据）：攻击 / 闪避状态、连击计数、爆气剩余时间。 */
USTRUCT()
struct FBlueBattleComboFragment : public FMassFragment
{
	GENERATED_BODY()

	/** 连击计数，攻击时取当前值作为段数后递增（表现层对蒙太奇数组长度取模） */
	int32 ComboIndex = 0;

	EAttackType LastAttackType = EAttackType::None;

	bool bAttacking = false;
	bool bEvasion = false;
	bool bComboWindowOpen = false;

	/** 爆气剩余秒数，仅在带 FBlueBattleBurstTag 时有效 */
	float BurstRemaining = 0.f;

	/** 闪避冷却剩余秒数，归零前不能再次闪避 */
	float DodgeCooldownRemaining = 0.f;

	/** 当前（最近一次）出招及其伤害，命中结算以此为准 */
	TWeakObjectPtr<UAnimMontage> CurrentSkill;
	float CurrentSkillDamage = 0.f;
};

/** 招式表：各类攻击的招式序列（以蒙太奇标识）与每招伤害，初值取自 UComboComponent。 */
USTRUCT()
struct FBlueBattleSkillSetFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UAnimMontage>> LightAttacks;

	UPROPERTY()
	TArray<TObjectPtr<UAnimMontage>> HeavyAttacks;

	UPROPERTY()
	TArray<TObjectPtr<UAnimMontage>> EXLightAttacks;

	UPROPERTY()
	TArray<TObjectPtr<UAnimMontage>> EXHeavyAttacks;

	UPROPERTY()
	TMap<TObjectPtr<UAnimMontage>, float> SkillDamage;

	const TArray<TObjectPtr<UAnimMontage>>& GetAttacks(EAttackType AttackType, bool bEX) const
	{
		if (AttackType == EAttackType::Heavy)
		{
			return bEX ? EXHeavyAttacks : HeavyAttacks;
		}
		return bEX ? EXLightAttacks : LightAttacks;
	}
};

template <>
struct TMassFragmentTraits<FBlueBattleSkillSetFragment> final
{
	enum
	{
		AuthorAcceptsItsNotTriviallyCopyable = true
	};
};

/** 连招 / 爆气 / 闪避配置，初值取自 UComboComponent。 */
USTRUCT()
struct FBlueBattleComboParamsFragment : public FMassFragment
{
	GENERATED_BODY()

	float BurstDuration = 15.f;
	float BurstEnergyCost = 50.f;
	float DodgeCooldown = 0.6f;
};

/** 爆气(EX)中 */
USTRUCT()
struct FBlueBattleBurstTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FBlueBattleComboComponentWrapperFragment : public FObjectWrapperFragment
{
	GENERATED_BODY()

	TWeakObjectPtr<UComboComponent> Component;
};

/** UBlueBattleComboToActorTranslator 的筛选标签 */
USTRUCT()
struct FBlueBattleComboCopyToActorTag : public FMassTag
{
	GENERATED_BODY()
};
