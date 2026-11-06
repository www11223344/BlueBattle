// Fill out your copyright notice in the Description page of Project Settings.

#include "GameModes/BlueBattleExperienceDefinition.h"

#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleExperienceDefinition)

#define LOCTEXT_NAMESPACE "BlueBattleExperience"

#if WITH_EDITOR
EDataValidationResult UBlueBattleExperienceDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	for (int32 Index = 0; Index < Actions.Num(); ++Index)
	{
		if (const UGameFeatureAction* Action = Actions[Index])
		{
			Result = CombineDataValidationResults(Result, Action->IsDataValid(Context));
		}
		else
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(Index)));
		}
	}

	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UBlueBattleExperienceDefinition::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
