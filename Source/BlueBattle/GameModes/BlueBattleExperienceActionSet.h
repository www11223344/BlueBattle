// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"

#include "BlueBattleExperienceActionSet.generated.h"

class UGameFeatureAction;

/** 可在多个 Experience 之间复用的一组 GameFeatureAction（对应 Lyra 的 ULyraExperienceActionSet） */
UCLASS(BlueprintType, NotBlueprintable)
class BLUEBATTLE_API UBlueBattleExperienceActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif

	UPROPERTY(EditAnywhere, Instanced, Category = "Actions to Perform")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	UPROPERTY(EditAnywhere, Category = "Feature Dependencies")
	TArray<FString> GameFeaturesToEnable;
};
