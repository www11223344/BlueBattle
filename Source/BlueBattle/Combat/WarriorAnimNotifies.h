// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "WarriorAnimNotifies.generated.h"

/**
 * 攻击判定窗口 Notify State：
 * 在蒙太奇上覆盖攻击生效帧段，NotifyBegin 打开攻击判定，NotifyEnd 关闭。
 */
UCLASS(meta = (DisplayName = "Attack Window"))
class BLUEBATTLE_API UAnimNotifyState_AttackWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** 该段攻击伤害倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageMultiplier = 1.f;
};

/**
 * 连招窗口 Notify State：
 * 在蒙太奇上覆盖“可续招”帧段，NotifyBegin 打开连招窗口，NotifyEnd 关闭。
 */
UCLASS(meta = (DisplayName = "Combo Window"))
class BLUEBATTLE_API UAnimNotifyState_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
