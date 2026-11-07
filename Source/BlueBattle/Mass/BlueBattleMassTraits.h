// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityTraitBase.h"
#include "BlueBattleMassTraits.generated.h"

/** 基础单位 Trait：为实体打上 FBlueBattleUnitTag。属性 / 连招等数据由各自的 Sync Trait 追加。 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "BlueBattle Unit"))
class BLUEBATTLE_API UBlueBattleUnitTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

/**
 * 属性同步 Trait：属性 Fragment 为权威数据，初值取自 Actor 上 UAttributeComponent 的配置，
 * 之后只经 UBlueBattleAttributeToActorTranslator 单向同步回组件。
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "BlueBattle Attribute Sync"))
class BLUEBATTLE_API UBlueBattleAttributeSyncTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

/**
 * 连招同步 Trait：连招 / 爆气状态机在 Fragment 中，配置取自 Actor 上的 UComboComponent，
 * 经 UBlueBattleComboToActorTranslator 单向同步回组件。
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "BlueBattle Combo Sync"))
class BLUEBATTLE_API UBlueBattleComboSyncTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
