// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGameStateFpsInterface.generated.h"

UINTERFACE()
class UGameStateFpsInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMELOCALSETTINGS_API IGameStateFpsInterface
{
	GENERATED_BODY()

public:
	virtual float GetServerFPS() const =0;
};
