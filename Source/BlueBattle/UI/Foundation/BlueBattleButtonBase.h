// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonButtonBase.h"
#include "UnLuaInterface.h"

#include "BlueBattleButtonBase.generated.h"

class UObject;
struct FFrame;

UCLASS(Abstract, BlueprintType, Blueprintable)
class BLUEBATTLE_API UBlueBattleButtonBase : public UCommonButtonBase, public IUnLuaInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetButtonText(const FText& InText);

	//~IUnLuaInterface
	virtual FString GetModuleName_Implementation() const override { return LuaModuleName; }
	//~End of IUnLuaInterface

protected:
	/** 绑定的 Lua 模块（相对 Content/Script，如 UI.W_Button1），为空则不绑定 */
	UPROPERTY(EditDefaultsOnly, Category = "Lua")
	FString LuaModuleName;
	
protected:
	// UUserWidget interface
	virtual void NativePreConstruct() override;
	// End of UUserWidget interface

	// UCommonButtonBase interface
	virtual void UpdateInputActionWidget() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	// End of UCommonButtonBase interface

	void RefreshButtonText();
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonText(const FText& InText);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonStyle();
	
private:
	UPROPERTY(EditAnywhere, Category="Button", meta=(InlineEditConditionToggle))
	uint8 bOverride_ButtonText : 1;
	
	UPROPERTY(EditAnywhere, Category="Button", meta=( editcondition="bOverride_ButtonText" ))
	FText ButtonText;
};
