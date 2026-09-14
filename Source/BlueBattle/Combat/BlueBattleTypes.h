// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueBattleTypes.generated.h"

/** 攻击类型：轻击 / 重击。对应输入映射 "轻击" / "重击"。 */
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	None    UMETA(DisplayName = "None"),
	Light   UMETA(DisplayName = "Light"),   // 轻击
	Heavy   UMETA(DisplayName = "Heavy")    // 重击
};

/**
 * 角色属性数据。可用于 DataTable / DataAsset 配置，也作为运行时数值来源。
 */
USTRUCT(BlueprintType)
struct FWarriorAttributes
{
	GENERATED_BODY()

	/** 最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxHealth = 100.f;

	/** 当前生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Health = 100.f;

	/** 最大魔法值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxMana = 100.f;

	/** 当前魔法值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Mana = 100.f;

	/** 最大能量值（爆气/EX 槽） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float MaxEnergy = 100.f;

	/** 当前能量值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Energy = 100.f;

	/** 基础攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float AttackPower = 20.f;

	/** 防御力（伤害减免）。设为 0 即为无减免（与原蓝图一致）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Defense = 0.f;
};

/**
 * 单段连招数据。可放进 DataTable 由蓝图配置动画/倍率。
 */
USTRUCT(BlueprintType)
struct FComboMoveData : public FTableRowBase
{
	GENERATED_BODY()

	/** 攻击类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	EAttackType AttackType = EAttackType::Light;

	/** 该段伤害倍率（最终伤害 = 攻击力 * 倍率 - 防御） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float DamageMultiplier = 1.f;

	/** 该段使用的攻击蒙太奇（表现层，蓝图/动画负责播放） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	TSoftObjectPtr<UAnimMontage> Montage;
};
