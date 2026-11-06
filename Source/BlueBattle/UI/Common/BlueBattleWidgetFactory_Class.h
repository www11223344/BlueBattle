// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BlueBattleWidgetFactory.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"

#include "BlueBattleWidgetFactory_Class.generated.h"

#define UE_API BLUEBATTLE_API

class UObject;
class UUserWidget;

UCLASS(MinimalAPI)
class UBlueBattleWidgetFactory_Class : public UBlueBattleWidgetFactory
{
	GENERATED_BODY()

public:
	UBlueBattleWidgetFactory_Class() { }

	UE_API virtual TSubclassOf<UUserWidget> FindWidgetClassForData_Implementation(const UObject* Data) const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = ListEntries, meta = (AllowAbstract))
	TMap<TSoftClassPtr<UObject>, TSubclassOf<UUserWidget>> EntryWidgetForClass;
};

#undef UE_API
