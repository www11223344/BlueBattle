// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "BlueBattleTabListWidgetBase.h"
#include "UI/Foundation/BlueBattleButtonBase.h"

#include "BlueBattleTabButtonBase.generated.h"

#define UE_API BLUEBATTLE_API

class UCommonLazyImage;
class UObject;
struct FFrame;
struct FSlateBrush;

UCLASS(MinimalAPI, Abstract, Blueprintable, meta = (DisableNativeTick))
class UBlueBattleTabButtonBase : public UBlueBattleButtonBase, public IBlueBattleTabButtonInterface
{
	GENERATED_BODY()

public:

	UE_API void SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject);
	UE_API void SetIconBrush(const FSlateBrush& Brush);

protected:

	UFUNCTION()
	UE_API virtual void SetTabLabelInfo_Implementation(const FBlueBattleTabDescriptor& TabLabelInfo) override;

private:

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonLazyImage> LazyImage_Icon;
};

#undef UE_API
