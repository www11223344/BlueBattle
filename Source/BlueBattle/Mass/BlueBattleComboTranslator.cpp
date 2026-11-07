// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/BlueBattleComboTranslator.h"
#include "Combat/ComboComponent.h"
#include "Mass/BlueBattleMassFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"

UBlueBattleComboToActorTranslator::UBlueBattleComboToActorTranslator()
	: EntityQuery(*this)
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::AllNetModes;
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::UpdateWorldFromMass;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Tasks);
	RequiredTags.Add<FBlueBattleComboCopyToActorTag>();
	bRequiresGameThreadExecution = true;
}

void UBlueBattleComboToActorTranslator::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AddRequiredTagsToQuery(EntityQuery);
	EntityQuery.AddRequirement<FBlueBattleComboComponentWrapperFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FBlueBattleComboFragment>(EMassFragmentAccess::ReadOnly);
}

void UBlueBattleComboToActorTranslator::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const bool bBursting = Context.DoesArchetypeHaveTag<FBlueBattleBurstTag>();
		const TArrayView<FBlueBattleComboComponentWrapperFragment> ComponentList = Context.GetMutableFragmentView<FBlueBattleComboComponentWrapperFragment>();
		const TConstArrayView<FBlueBattleComboFragment> ComboList = Context.GetFragmentView<FBlueBattleComboFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (UComboComponent* ComboComponent = ComponentList[EntityIt].Component.Get())
			{
				ComboComponent->ApplyMassSnapshot(ComboList[EntityIt], bBursting);
			}
		}
	});
}
