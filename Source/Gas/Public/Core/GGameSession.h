// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSession/BaseGameSession.h"
#include "GGameSession.generated.h"

UCLASS()
class GAS_API AGGameSession : public ABaseGameSession
{
	GENERATED_BODY()

public:
	AGGameSession(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	{
	}

};
