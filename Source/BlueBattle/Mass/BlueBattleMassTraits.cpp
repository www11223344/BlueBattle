// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/BlueBattleMassTraits.h"
#include "BlueBattleLogChannels.h"
#include "Characters/WarriorCharacter.h"
#include "Combat/AttributeComponent.h"
#include "Combat/ComboComponent.h"
#include "Mass/BlueBattleAttributeTranslator.h"
#include "Mass/BlueBattleComboTranslator.h"
#include "Mass/BlueBattleMassFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityView.h"

void UBlueBattleUnitTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddTag<FBlueBattleUnitTag>();
}

void UBlueBattleAttributeSyncTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FBlueBattleAttributeFragment>();
	BuildContext.AddFragment<FBlueBattleAttributeRegenFragment>();
	BuildContext.AddFragment<FBlueBattleAttributeComponentWrapperFragment>();
	BuildContext.AddTranslator<UBlueBattleAttributeToActorTranslator>();

	BuildContext.GetMutableObjectFragmentInitializers().Add([](UObject& Owner, FMassEntityView& EntityView, const EMassTranslationDirection CurrentDirection)
	{
		UAttributeComponent* AttributeComponent = nullptr;
		if (const AWarriorCharacter* Warrior = Cast<AWarriorCharacter>(&Owner))
		{
			AttributeComponent = Warrior->GetAttributeComponent();
		}
		else if (const AActor* Actor = Cast<AActor>(&Owner))
		{
			AttributeComponent = Actor->FindComponentByClass<UAttributeComponent>();
		}

		if (!AttributeComponent)
		{
			UE_LOG(LogBlueBattle, Error, TEXT("%s has no UAttributeComponent, attribute fragments keep defaults"), *Owner.GetName());
			return;
		}

		EntityView.GetFragmentData<FBlueBattleAttributeComponentWrapperFragment>().Component = AttributeComponent;
		AttributeComponent->ExportToMass(EntityView.GetFragmentData<FBlueBattleAttributeFragment>(), EntityView.GetFragmentData<FBlueBattleAttributeRegenFragment>());
		AttributeComponent->BindMassEntity(EntityView.GetEntity());
	});
}

void UBlueBattleComboSyncTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FBlueBattleComboFragment>();
	BuildContext.AddFragment<FBlueBattleComboParamsFragment>();
	BuildContext.AddFragment<FBlueBattleSkillSetFragment>();
	BuildContext.AddFragment<FBlueBattleComboComponentWrapperFragment>();
	BuildContext.AddTranslator<UBlueBattleComboToActorTranslator>();

	BuildContext.GetMutableObjectFragmentInitializers().Add([](UObject& Owner, FMassEntityView& EntityView, const EMassTranslationDirection CurrentDirection)
	{
		UComboComponent* ComboComponent = nullptr;
		if (const AWarriorCharacter* Warrior = Cast<AWarriorCharacter>(&Owner))
		{
			ComboComponent = Warrior->GetComboComponent();
		}
		else if (const AActor* Actor = Cast<AActor>(&Owner))
		{
			ComboComponent = Actor->FindComponentByClass<UComboComponent>();
		}

		if (!ComboComponent)
		{
			UE_LOG(LogBlueBattle, Error, TEXT("%s has no UComboComponent, combo fragments keep defaults"), *Owner.GetName());
			return;
		}

		EntityView.GetFragmentData<FBlueBattleComboComponentWrapperFragment>().Component = ComboComponent;
		ComboComponent->ExportToMass(EntityView.GetFragmentData<FBlueBattleComboParamsFragment>(), EntityView.GetFragmentData<FBlueBattleSkillSetFragment>());
		ComboComponent->BindMassEntity(EntityView.GetEntity());
	});
}
