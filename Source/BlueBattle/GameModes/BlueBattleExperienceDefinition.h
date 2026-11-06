// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"

#include "BlueBattleExperienceDefinition.generated.h"

class UGameFeatureAction;
class UBlueBattleExperienceActionSet;
class UBlueBattlePawnData;

/**
 * 对应 Lyra 的 ULyraExperienceDefinition：描述进入某张地图后要加载的玩法配置。
 * 地图通过 ABlueBattleWorldSettings::DefaultGameplayExperience 引用它。
 */
UCLASS(BlueprintType, Const)
class BLUEBATTLE_API UBlueBattleExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif

	/** 需要激活的 GameFeature 插件 */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TArray<FString> GameFeaturesToEnable;

	/** 玩家默认使用的 PawnData；为空时使用 GameMode 的 DefaultPawnClass */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TObjectPtr<const UBlueBattlePawnData> DefaultPawnData;

	/** 本地玩家数量（分屏人数）；加载完成后多删少补 */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay", meta = (ClampMin = 1, ClampMax = 4))
	int32 NumLocalPlayers = 1;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	TArray<TObjectPtr<UBlueBattleExperienceActionSet>> ActionSets;
};
