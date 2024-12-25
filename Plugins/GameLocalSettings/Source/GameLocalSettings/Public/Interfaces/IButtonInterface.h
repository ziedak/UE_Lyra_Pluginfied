// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "IButtonInterface.generated.h"

UINTERFACE()
class UButtonInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMELOCALSETTINGS_API IButtonInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Button Interface")
	void SetButtonText(const FText& InText);
	virtual void SetButtonText_Implementation(const FText& InText) {};
};
