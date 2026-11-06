// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/BlueBattleExperienceActionSet.h"

#include "GameFeatureAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleExperienceActionSet)

#if WITH_EDITORONLY_DATA
void UBlueBattleExperienceActionSet::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif
