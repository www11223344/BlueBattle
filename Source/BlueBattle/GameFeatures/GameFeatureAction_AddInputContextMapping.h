// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "UObject/SoftObjectPtr.h"

#include "GameFeatureAction_AddInputContextMapping.generated.h"

class APlayerController;
class UInputMappingContext;
struct FComponentRequestHandle;

USTRUCT(BlueprintType)
struct FBlueBattleInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AssetBundles = "Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	/** 优先级高的映射先处理 */
	UPROPERTY(EditAnywhere, Category = "Input")
	int32 Priority = 0;
};

/**
 * 在 HeroComponent 发出 BindInputsNow 时，把映射上下文加到该玩家的 EnhancedInput 子系统
 * （对应 Lyra 的 UGameFeatureAction_AddInputContextMapping，去掉了按键设置注册）。
 */
UCLASS(BlueprintType, meta = (DisplayName = "Add Input Mapping"))
class BLUEBATTLE_API UGameFeatureAction_AddInputContextMapping final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction interface

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FBlueBattleInputMappingContextAndPriority> InputMappings;

private:
	struct FPerContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		TArray<TWeakObjectPtr<APlayerController>> ControllersAddedTo;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

	void Reset(FPerContextData& ActiveData);
	void HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext);
	void AddInputMapping(APlayerController* PlayerController, FPerContextData& ActiveData);
	void RemoveInputMapping(APlayerController* PlayerController, FPerContextData& ActiveData);
};
