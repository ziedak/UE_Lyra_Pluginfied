// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameState/PlayerSpawningManagerComponent.h"
#include "UObject/Interface.h"
#include "SpawnInterface.generated.h"


UINTERFACE()
class USpawnInterface : public UInterface
{
	GENERATED_BODY()
};

class CUSTOMCORE_API ISpawnInterface
{
	GENERATED_BODY()

public:
	virtual bool TryClaim(AController* Player) =0;
	virtual EPlayerStartLocationOccupancy GetLocationOccupancy(AController* Controller) =0;
};
