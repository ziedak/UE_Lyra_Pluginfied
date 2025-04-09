// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonButtonBase.h"

#include "Interfaces/IButtonInterface.h"

#include "LyraButtonBase.generated.h"

class UObject;
struct FFrame;

UCLASS(Abstract, BlueprintType, Blueprintable)
class ULyraButtonBase : public UCommonButtonBase, public IButtonInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void SetButtonText(const FText& InText) override;

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