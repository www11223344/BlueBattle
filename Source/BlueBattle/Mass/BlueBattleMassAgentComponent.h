// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassAgentComponent.h"
#include "BlueBattleMassAgentComponent.generated.h"

class UBlueBattleMassAgentComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBlueBattleMassEntityChanged, UBlueBattleMassAgentComponent* /*AgentComponent*/);

/**
 * 单位与 MassEntity 的绑定组件：Actor 注册时由 UMassAgentSubsystem 创建实体，
 * 实体承载数据层（Fragment），Actor 及其组件只做表现，由 Translator 单向同步。
 *
 * 构造时默认装配 UBlueBattleUnitTrait，蓝图可在 EntityConfig 中追加其它 Trait。
 */
UCLASS(ClassGroup = (BlueBattle), meta = (BlueprintSpawnableComponent))
class BLUEBATTLE_API UBlueBattleMassAgentComponent : public UMassAgentComponent
{
	GENERATED_BODY()

public:
	UBlueBattleMassAgentComponent();

	/** 实体绑定后广播（实体由 UMassAgentSubsystem 在下一次 Mass 处理阶段异步创建，BeginPlay 时可能还没有） */
	FOnBlueBattleMassEntityChanged OnEntityChanged;

	UFUNCTION(BlueprintPure, Category = "Mass")
	bool HasMassEntity() const { return GetEntityHandle().IsValid(); }

protected:
	virtual void OnRegister() override;
	virtual void SetEntityHandleInternal(const FMassEntityHandle NewHandle) override;

private:
	void EnsureTrait(TSubclassOf<UMassEntityTraitBase> TraitClass);
};
