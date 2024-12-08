// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameStateFps.generated.h"

UINTERFACE()
class UGameStateFps : public UInterface
{
	GENERATED_BODY()
};

class GAMELOCALSETTINGS_API IGameStateFps
{
	GENERATED_BODY()

public:
	virtual float GetServerFPS() const =0;
};
