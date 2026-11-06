// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/BlueBattleWorldSettings.h"

#include "BlueBattleLogChannels.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleWorldSettings)

ABlueBattleWorldSettings::ABlueBattleWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FPrimaryAssetId ABlueBattleWorldSettings::GetDefaultGameplayExperience() const
{
	FPrimaryAssetId Result;
	if (!DefaultGameplayExperience.IsNull())
	{
		Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());
		if (!Result.IsValid())
		{
			UE_LOG(LogBlueBattleExperience, Error, TEXT("%s.DefaultGameplayExperience is %s but it is not a scanned primary asset (check AssetManager PrimaryAssetTypesToScan)"),
				*GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
		}
	}
	return Result;
}
