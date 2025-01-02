// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Hud/LyraGameViewportClient.h"
#include "BaseGameViewportClient.generated.h"

UCLASS(BlueprintType)
class GAS_API UBaseGameViewportClient : public ULyraGameViewportClient
{
	GENERATED_BODY()

public :
	UBaseGameViewportClient(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()): Super(
		ObjectInitializer) {}
};
