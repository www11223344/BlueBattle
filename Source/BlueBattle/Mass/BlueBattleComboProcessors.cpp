// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/BlueBattleComboProcessors.h"
#include "Mass/BlueBattleMassFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"

UBlueBattleComboStateProcessor::UBlueBattleComboStateProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UBlueBattleComboStateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FBlueBattleComboFragment>(EMassFragmentAccess::ReadWrite);
}

void UBlueBattleComboStateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const float DeltaTime = Context.GetDeltaTimeSeconds();
		const TArrayView<FBlueBattleComboFragment> ComboList = Context.GetMutableFragmentView<FBlueBattleComboFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FBlueBattleComboFragment& Combo = ComboList[EntityIt];
			if (Combo.bAttacking && Combo.bEvasion)
			{
				Combo.bAttacking = false;
				Combo.bEvasion = false;
			}
			Combo.DodgeCooldownRemaining = FMath::Max(0.f, Combo.DodgeCooldownRemaining - DeltaTime);
		}
	});
}

UBlueBattleBurstTimerProcessor::UBlueBattleBurstTimerProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UBlueBattleBurstTimerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FBlueBattleComboFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FBlueBattleBurstTag>(EMassFragmentPresence::All);
}

void UBlueBattleBurstTimerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const float DeltaTime = Context.GetDeltaTimeSeconds();
		const TArrayView<FBlueBattleComboFragment> ComboList = Context.GetMutableFragmentView<FBlueBattleComboFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FBlueBattleComboFragment& Combo = ComboList[EntityIt];
			Combo.BurstRemaining -= DeltaTime;
			if (Combo.BurstRemaining <= 0.f)
			{
				Combo.BurstRemaining = 0.f;
				Context.Defer().RemoveTag<FBlueBattleBurstTag>(Context.GetEntity(EntityIt));
			}
		}
	});
}
