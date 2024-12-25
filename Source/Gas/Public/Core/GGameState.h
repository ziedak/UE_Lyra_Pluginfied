// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameState/BaseGameState.h"
#include "GGameState.generated.h"


UCLASS()
class GAS_API AGGameState : public ABaseGameState
{
	GENERATED_BODY()


#pragma region AGameStateBase interface
	virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
#pragma endregion AGameStateBase interface
};
