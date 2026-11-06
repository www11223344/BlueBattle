// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModels/BlueBattleLocalPlayerViewModelResolver.h"

#include "Blueprint/UserWidget.h"
#include "UI/ViewModels/BlueBattleAttributeViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleLocalPlayerViewModelResolver)

UObject* UBlueBattleLocalPlayerViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	if (!UserWidget || !ExpectedType || !ExpectedType->IsChildOf(UBlueBattleAttributeViewModel::StaticClass()))
	{
		return nullptr;
	}

	UUserWidget* OwningWidget = const_cast<UUserWidget*>(UserWidget);
	UBlueBattleAttributeViewModel* ViewModel = NewObject<UBlueBattleAttributeViewModel>(OwningWidget, ExpectedType);
	ViewModel->InitializeForLocalPlayer(OwningWidget->GetOwningLocalPlayer());
	return ViewModel;
}

void UBlueBattleLocalPlayerViewModelResolver::DestroyInstance(UObject* ViewModel, const UMVVMView* View) const
{
	if (UBlueBattleAttributeViewModel* AttributeViewModel = Cast<UBlueBattleAttributeViewModel>(ViewModel))
	{
		AttributeViewModel->Deinitialize();
	}
}

#if WITH_EDITOR
bool UBlueBattleLocalPlayerViewModelResolver::DoesSupportViewModelClass(const UClass* Class) const
{
	return Class && Class->IsChildOf(UBlueBattleAttributeViewModel::StaticClass());
}
#endif
