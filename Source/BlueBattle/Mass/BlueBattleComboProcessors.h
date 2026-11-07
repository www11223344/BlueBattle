// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassEntityQuery.h"
#include "MassProcessor.h"
#include "BlueBattleComboProcessors.generated.h"

/** 攻击与闪避互斥：同一帧两者同时为真时都清除（原 ComboSystem Tick 规则）；闪避冷却倒计时。 */
UCLASS()
class BLUEBATTLE_API UBlueBattleComboStateProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UBlueBattleComboStateProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

/** 爆气倒计时，到时移除 FBlueBattleBurstTag。 */
UCLASS()
class BLUEBATTLE_API UBlueBattleBurstTimerProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UBlueBattleBurstTimerProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
