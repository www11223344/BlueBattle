// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "BlueBattleAttributeProcessors.generated.h"

/** 存活单位每帧：MP 向上限匀速回复，能量向 0 匀速衰减。 */
UCLASS()
class BLUEBATTLE_API UBlueBattleAttributeRegenProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UBlueBattleAttributeRegenProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/** HP 归零的单位打上 FBlueBattleDeadTag（伤害写入通道也会即时打标签，这里兜底其它扣血来源）。 */
UCLASS()
class BLUEBATTLE_API UBlueBattleDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UBlueBattleDeathProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
