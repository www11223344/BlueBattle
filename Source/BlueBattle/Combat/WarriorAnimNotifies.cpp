// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/WarriorAnimNotifies.h"
#include "Combat/CombatComponent.h"
#include "Combat/ComboComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotifyState_AttackWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UCombatComponent>())
		{
			Combat->BeginAttackWindow(DamageMultiplier);
		}
	}
}

void UAnimNotifyState_AttackWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UCombatComponent>())
		{
			Combat->EndAttackWindow();
		}
	}
}

void UAnimNotifyState_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UComboComponent* Combo = MeshComp->GetOwner()->FindComponentByClass<UComboComponent>())
		{
			Combo->OpenComboWindow();
		}
	}
}

void UAnimNotifyState_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UComboComponent* Combo = MeshComp->GetOwner()->FindComponentByClass<UComboComponent>())
		{
			Combo->CloseComboWindow();
		}
	}
}
