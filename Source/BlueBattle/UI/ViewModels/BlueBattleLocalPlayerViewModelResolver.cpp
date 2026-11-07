// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ViewModels/BlueBattleLocalPlayerViewModelResolver.h"

#include "Blueprint/UserWidget.h"
#include "UI/ViewModels/BlueBattleAttributeViewModel.h"
#include "UI/ViewModels/BlueBattleBurstViewModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleLocalPlayerViewModelResolver)

UObject* UBlueBattleLocalPlayerViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	if (!UserWidget || !ExpectedType)
	{
		return nullptr;
	}

	UUserWidget* OwningWidget = const_cast<UUserWidget*>(UserWidget);
	if (ExpectedType->IsChildOf(UBlueBattleAttributeViewModel::StaticClass()))
	{
		UBlueBattleAttributeViewModel* ViewModel = NewObject<UBlueBattleAttributeViewModel>(OwningWidget, ExpectedType);
		ViewModel->InitializeForLocalPlayer(OwningWidget->GetOwningLocalPlayer());
		return ViewModel;
	}
	if (ExpectedType->IsChildOf(UBlueBattleBurstViewModel::StaticClass()))
	{
		UBlueBattleBurstViewModel* ViewModel = NewObject<UBlueBattleBurstViewModel>(OwningWidget, ExpectedType);
		ViewModel->InitializeForLocalPlayer(OwningWidget->GetOwningLocalPlayer());
		return ViewModel;
	}
	return nullptr;
}

void UBlueBattleLocalPlayerViewModelResolver::DestroyInstance(UObject* ViewModel, const UMVVMView* View) const
{
	if (UBlueBattleAttributeViewModel* AttributeViewModel = Cast<UBlueBattleAttributeViewModel>(ViewModel))
	{
		AttributeViewModel->Deinitialize();
	}
	else if (UBlueBattleBurstViewModel* BurstViewModel = Cast<UBlueBattleBurstViewModel>(ViewModel))
	{
		BurstViewModel->Deinitialize();
	}
}

#if WITH_EDITOR
bool UBlueBattleLocalPlayerViewModelResolver::DoesSupportViewModelClass(const UClass* Class) const
{
	return Class && (Class->IsChildOf(UBlueBattleAttributeViewModel::StaticClass()) || Class->IsChildOf(UBlueBattleBurstViewModel::StaticClass()));
}
#endif
