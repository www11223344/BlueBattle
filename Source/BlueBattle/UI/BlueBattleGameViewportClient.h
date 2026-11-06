// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameViewportClient.h"

#include "BlueBattleGameViewportClient.generated.h"

class UGameInstance;
class UObject;

UCLASS(BlueprintType)
class UBlueBattleGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	UBlueBattleGameViewportClient();

	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
};
