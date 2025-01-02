// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/BaseGameMode.h"
#include "GGameMode.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGGameMode : public ABaseGameMode
{
	GENERATED_BODY()

public:
	AGGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
	virtual const UGasPawnData* GetPawnDataForController(const AController* InController) const;

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	virtual APawn*
	SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
};
