// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "ButtonInterface.generated.h"

UINTERFACE()
class UButtonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAMELOCALSETTINGS_API IButtonInterface
{
	GENERATED_BODY()

public:
	virtual void SetButtonText(const FText& InText)=0;
};
