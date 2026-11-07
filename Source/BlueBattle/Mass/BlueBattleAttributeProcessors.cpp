// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/BlueBattleAttributeProcessors.h"
#include "Mass/BlueBattleMassFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"

UBlueBattleAttributeRegenProcessor::UBlueBattleAttributeRegenProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
}

void UBlueBattleAttributeRegenProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FBlueBattleAttributeFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FBlueBattleAttributeRegenFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FBlueBattleDeadTag>(EMassFragmentPresence::None);
}

void UBlueBattleAttributeRegenProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const float DeltaTime = Context.GetDeltaTimeSeconds();
		const TArrayView<FBlueBattleAttributeFragment> AttributeList = Context.GetMutableFragmentView<FBlueBattleAttributeFragment>();
		const TConstArrayView<FBlueBattleAttributeRegenFragment> RegenList = Context.GetFragmentView<FBlueBattleAttributeRegenFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FBlueBattleAttributeFragment& Attributes = AttributeList[EntityIt];
			const FBlueBattleAttributeRegenFragment& Regen = RegenList[EntityIt];
			Attributes.MP = FMath::FInterpConstantTo(Attributes.MP, Attributes.MaxMP, DeltaTime, Regen.MPRegenSpeed);
			Attributes.Energy = FMath::FInterpConstantTo(Attributes.Energy, 0.f, DeltaTime, Regen.EnergyDecaySpeed);
		}
	});
}

UBlueBattleDeathProcessor::UBlueBattleDeathProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteAfter.Add(UBlueBattleAttributeRegenProcessor::StaticClass()->GetFName());
}

void UBlueBattleDeathProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FBlueBattleAttributeFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FBlueBattleDeadTag>(EMassFragmentPresence::None);
}

void UBlueBattleDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FBlueBattleAttributeFragment> AttributeList = Context.GetFragmentView<FBlueBattleAttributeFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (AttributeList[EntityIt].HP <= 0.f)
			{
				Context.Defer().AddTag<FBlueBattleDeadTag>(Context.GetEntity(EntityIt));
			}
		}
	});
}
