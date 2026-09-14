// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/WarriorCharacter.h"
#include "Combat/AttributeComponent.h"
#include "Combat/CombatComponent.h"
#include "Combat/ComboComponent.h"
#include "Engine/World.h"

AWarriorCharacter::AWarriorCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// CombatComponent 为 C++ 新增的命中检测组件（蓝图无对应），由本类创建。
	// AttributeComponent / ComboComponent 复用蓝图已有的 AttributeSystem / ComboSystem
	// （它们已 Reparent 到 C++ 基类），在 BeginPlay 中按类查找，避免重复挂载两套组件。
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

void AWarriorCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 查找蓝图挂载的属性/连招组件（AttributeSystem / ComboSystem，已继承 C++ 基类）
	AttributeComponent = FindComponentByClass<UAttributeComponent>();
	ComboComponent = FindComponentByClass<UComboComponent>();

	if (ComboComponent)
	{
		ComboComponent->OnComboStepChanged.AddDynamic(this, &AWarriorCharacter::HandleComboStepChanged);
	}
	if (CombatComponent)
	{
		CombatComponent->OnHitTarget.AddDynamic(this, &AWarriorCharacter::HandleHitTarget);
	}
	if (AttributeComponent)
	{
		AttributeComponent->OnDeath.AddDynamic(this, &AWarriorCharacter::HandleDeath);
	}
}

void AWarriorCharacter::LightAttack()
{
	if (ComboComponent && IsAlive())
	{
		ComboComponent->PerformLightAttack();
	}
}

void AWarriorCharacter::HeavyAttack()
{
	if (ComboComponent && IsAlive())
	{
		ComboComponent->PerformHeavyAttack();
	}
}

void AWarriorCharacter::Dodge()
{
	if (!ComboComponent || !IsAlive())
	{
		return;
	}

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (Now - LastDodgeTime < DodgeCooldown)
	{
		return;
	}
	LastDodgeTime = Now;

	ComboComponent->PerformDodge();
	OnDodgePerformed();
}

void AWarriorCharacter::Burst()
{
	if (ComboComponent && IsAlive())
	{
		ComboComponent->ActivateBurst();
	}
}

bool AWarriorCharacter::IsAttacking() const
{
	return ComboComponent ? ComboComponent->IsAttacking() : false;
}

int32 AWarriorCharacter::GetComboStep() const
{
	return ComboComponent ? ComboComponent->GetComboCount() : 0;
}

bool AWarriorCharacter::IsAlive() const
{
	return AttributeComponent ? AttributeComponent->IsAlive() : true;
}

void AWarriorCharacter::HandleComboStepChanged(int32 ComboStep, EAttackType AttackType)
{
	// 转发到蓝图，由蓝图播放对应蒙太奇
	OnAttackPerformed(ComboStep, AttackType);
}

void AWarriorCharacter::HandleHitTarget(AActor* HitActor, float DamageDealt, const FHitResult& HitResult)
{
	// 通知被击中方播放受击表现
	if (AWarriorCharacter* HitWarrior = Cast<AWarriorCharacter>(HitActor))
	{
		HitWarrior->OnHitReaction(DamageDealt, this);
	}
}

void AWarriorCharacter::HandleDeath(AActor* Killer)
{
	OnDeathFeedback(Killer);
}
