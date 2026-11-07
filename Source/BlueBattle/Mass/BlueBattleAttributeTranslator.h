// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityQuery.h"
#include "MassTranslator.h"
#include "BlueBattleAttributeTranslator.generated.h"

/** 属性 Fragment → UAttributeComponent 的单向同步：组件只做镜像，值变化时由组件广播 OnHPChanged 等委托。 */
UCLASS()
class BLUEBATTLE_API UBlueBattleAttributeToActorTranslator : public UMassTranslator
{
	GENERATED_BODY()

public:
	UBlueBattleAttributeToActorTranslator();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
