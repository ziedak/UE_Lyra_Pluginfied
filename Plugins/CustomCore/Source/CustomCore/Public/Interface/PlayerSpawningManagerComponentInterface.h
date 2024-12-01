// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerSpawningManagerComponentInterface.generated.h"


UINTERFACE()
class UPlayerSpawningManagerComponentInterface : public UInterface
{
	GENERATED_BODY()
};

class CUSTOMCORE_API IPlayerSpawningManagerComponentInterface
{
	GENERATED_BODY()

public:
	virtual bool ControllerCanRestart(AController* Controller) =0;
	virtual void FinishRestartPlayer(AController* Controller, const FRotator& Rotator) =0;
	virtual AActor* ChoosePlayerStart(AController* Player) =0;
};
