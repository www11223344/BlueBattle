// Fill out your copyright notice in the Description page of Project Settings.

#include "System/BlueBattleGameInstance.h"

#include "BlueBattleGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleGameInstance)

UBlueBattleGameInstance::UBlueBattleGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBlueBattleGameInstance::Init()
{
	Super::Init();

	// PawnExtension/Hero 组件按此顺序推进初始化
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);
	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(BlueBattleGameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(BlueBattleGameplayTags::InitState_DataAvailable, false, BlueBattleGameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(BlueBattleGameplayTags::InitState_DataInitialized, false, BlueBattleGameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(BlueBattleGameplayTags::InitState_GameplayReady, false, BlueBattleGameplayTags::InitState_DataInitialized);
	}
}
