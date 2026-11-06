// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueBattleWidgetFactory.h"
#include "Templates/SubclassOf.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleWidgetFactory)

class UUserWidget;

TSubclassOf<UUserWidget> UBlueBattleWidgetFactory::FindWidgetClassForData_Implementation(const UObject* Data) const
{
	return TSubclassOf<UUserWidget>();
}
