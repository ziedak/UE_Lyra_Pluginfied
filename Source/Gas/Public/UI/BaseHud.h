// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHud.generated.h"

/**
 *
 *  Note that you typically do not need to extend or modify this class, instead you would
 *  use an "Add Widget" action in your experience to add a HUD layout and widgets to it
 * 
 *  This class exists primarily for debug rendering
 */
UCLASS(Config = Game)
class GAS_API ABaseHud : public AHUD
{
	GENERATED_BODY()

public:
	ABaseHud(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()): Super(ObjectInitializer) {}

protected:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
};
