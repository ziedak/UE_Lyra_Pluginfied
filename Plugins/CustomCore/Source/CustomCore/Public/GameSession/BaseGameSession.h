// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "BaseGameSession.generated.h"

UCLASS(Config = Game)
class CUSTOMCORE_API ABaseGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	ABaseGameSession(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
	}

protected:
	/** Override to disable the default behavior */
	virtual bool ProcessAutoLogin() override
	{
		// This is actually handled in LyraGameMode::TryDedicatedServerLogin
		return true;
	}
};
