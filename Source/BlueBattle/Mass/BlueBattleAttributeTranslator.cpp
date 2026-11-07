// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/BlueBattleAttributeTranslator.h"
#include "Combat/AttributeComponent.h"
#include "Mass/BlueBattleMassFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"

UBlueBattleAttributeToActorTranslator::UBlueBattleAttributeToActorTranslator()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Tasks);
	RequiredTags.Add<FBlueBattleAttributeCopyToActorTag>();
	bRequiresGameThreadExecution = true;
}

void UBlueBattleAttributeToActorTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FBlueBattleAttributeComponentWrapperFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FBlueBattleAttributeFragment>(EMassFragmentAccess::ReadOnly);
}

void UBlueBattleAttributeToActorTranslator::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const bool bChunkDead = Context.DoesArchetypeHaveTag<FBlueBattleDeadTag>();
		const TArrayView<FBlueBattleAttributeComponentWrapperFragment> ComponentList = Context.GetMutableFragmentView<FBlueBattleAttributeComponentWrapperFragment>();
		const TConstArrayView<FBlueBattleAttributeFragment> AttributeList = Context.GetFragmentView<FBlueBattleAttributeFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (UAttributeComponent* AttributeComponent = ComponentList[EntityIt].Component.Get())
			{
				const FBlueBattleAttributeFragment& Attributes = AttributeList[EntityIt];
				AttributeComponent->ApplyMassSnapshot(Attributes, bChunkDead || Attributes.HP <= 0.f);
			}
		}
	});
}
