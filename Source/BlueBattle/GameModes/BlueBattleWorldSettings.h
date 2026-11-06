// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/WorldSettings.h"

#include "BlueBattleWorldSettings.generated.h"

class UBlueBattleExperienceDefinition;

/** 地图的 WorldSettings：指定该地图默认加载的 Experience */
UCLASS()
class BLUEBATTLE_API ABlueBattleWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	ABlueBattleWorldSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	UPROPERTY(EditAnywhere, Category = "GameMode")
	TSoftObjectPtr<UBlueBattleExperienceDefinition> DefaultGameplayExperience;
};
