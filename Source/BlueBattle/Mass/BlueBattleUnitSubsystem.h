// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Combat/BlueBattleTypes.h"
#include "Mass/EntityHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "BlueBattleUnitSubsystem.generated.h"

class UAnimMontage;
struct FBlueBattleAttributeFragment;
struct FBlueBattleComboFragment;
struct FMassEntityManager;

/**
 * 单位数据层的写入通道：表现层（组件 / 蓝图 / 命中检测）对属性与连招状态的所有修改都经由这里落到 Mass Fragment，
 * Fragment 再由 Translator 单向同步回组件。须在游戏线程调用。
 */
UCLASS()
class BLUEBATTLE_API UBlueBattleUnitSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 应用伤害（扣除防御），返回实际扣除的血量；HP 归零时立即标记死亡。 */
	float ApplyDamage(FMassEntityHandle Entity, float RawDamage, AActor* DamageCauser);

	void Heal(FMassEntityHandle Entity, float Amount);

	void ConsumeMP(FMassEntityHandle Entity, float Cost);

	/** 能量不足时不扣除并返回 false。 */
	bool ConsumeEnergy(FMassEntityHandle Entity, float Cost);

	void AddEnergy(FMassEntityHandle Entity, float Amount);

	/** 直接设置生命值（调试用）：超过上限时同时抬高上限，设为 0 时标记死亡；已死亡的单位不处理并返回 false。 */
	bool SetHP(FMassEntityHandle Entity, float NewHP);

	/** 设置该单位每次命中的固定伤害（调试用），小于 0 恢复按招式表结算。 */
	bool SetDamageOverride(FMassEntityHandle Entity, float Damage);

	const FBlueBattleAttributeFragment* GetAttributes(FMassEntityHandle Entity) const;

	bool IsDead(FMassEntityHandle Entity) const;

	// ---- 连招 / 爆气 ----

	/**
	 * 存活、不在攻击 / 闪避中且该类招式序列非空才能出招。成功时递增连击计数，
	 * 按段数从招式表选出本招并记录其伤害，返回段序号、是否为 EX 段与本招蒙太奇。
	 */
	bool TryStartAttack(FMassEntityHandle Entity, EAttackType AttackType, int32& OutComboStep, bool& bOutEX, UAnimMontage*& OutSkill);

	/**
	 * 攻击判定命中目标：攻击者存活且处于攻击中、目标为其他存活单位时，
	 * 以当前招式伤害（攻击者设置了 DamageOverride 时以其为准）结算到目标，并为攻击者增加等量能量。
	 * 返回目标实际损失的生命（未命中为 0）。
	 */
	float ResolveHit(FMassEntityHandle Attacker, FMassEntityHandle Target, AActor* DamageCauser);

	/** 存活、不在闪避中且冷却结束才能闪避；闪避打断攻击并进入冷却。 */
	bool TryStartDodge(FMassEntityHandle Entity);

	void EndAttack(FMassEntityHandle Entity);

	/** 闪避结束同时结束攻击状态 */
	void EndDodge(FMassEntityHandle Entity);

	void ResetCombo(FMassEntityHandle Entity);

	void SetComboWindowOpen(FMassEntityHandle Entity, bool bOpen);

	/** 能量达到 EX 阈值且不在爆气中时，消耗能量进入爆气。 */
	bool TryActivateBurst(FMassEntityHandle Entity);

	bool IsBursting(FMassEntityHandle Entity) const;

private:
	FMassEntityManager* GetEntityManager() const;
	FBlueBattleAttributeFragment* GetMutableAttributes(FMassEntityHandle Entity) const;
	FBlueBattleComboFragment* GetMutableCombo(FMassEntityHandle Entity) const;
	void MarkDead(FMassEntityHandle Entity);
};
