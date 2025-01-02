// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hud/LyraHUD.h"

// #include "GameFramework/HUD.h"
#include "BaseHud.generated.h"

/**
 *
 *  Note that you typically do not need to extend or modify this class, instead you would
 *  use an "Add Widget" action in your experience to add a HUD layout and widgets to it
 * 
 *  This class exists primarily for debug rendering
 */
UCLASS(Config = Game)
class GAS_API ABaseHud : public ALyraHUD
{
	GENERATED_BODY()

public:
	ABaseHud(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()): Super(ObjectInitializer) {}

protected:
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
};
