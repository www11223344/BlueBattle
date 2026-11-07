// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityQuery.h"
#include "MassTranslator.h"
#include "BlueBattleComboTranslator.generated.h"

/** 连招 Fragment → UComboComponent 的单向同步：组件镜像状态，并在状态变化时广播事件、切换移动速度。 */
UCLASS()
class BLUEBATTLE_API UBlueBattleComboToActorTranslator : public UMassTranslator
{
	GENERATED_BODY()

public:
	UBlueBattleComboToActorTranslator();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
