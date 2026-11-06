// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueBattleTabButtonBase.h"

#include "CommonLazyImage.h"
#include "UI/Common/BlueBattleTabListWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueBattleTabButtonBase)

class UObject;
struct FSlateBrush;

void UBlueBattleTabButtonBase::SetIconFromLazyObject(TSoftObjectPtr<UObject> LazyObject)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrushFromLazyDisplayAsset(LazyObject);
	}
}

void UBlueBattleTabButtonBase::SetIconBrush(const FSlateBrush& Brush)
{
	if (LazyImage_Icon)
	{
		LazyImage_Icon->SetBrush(Brush);
	}
}

void UBlueBattleTabButtonBase::SetTabLabelInfo_Implementation(const FBlueBattleTabDescriptor& TabLabelInfo)
{
	SetButtonText(TabLabelInfo.TabText);
	SetIconBrush(TabLabelInfo.IconBrush);
}

