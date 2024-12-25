// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonGameViewportClient.h"
#include "BaseGameViewportClient.generated.h"

UCLASS(BlueprintType)
class GAS_API UBaseGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UBaseGameViewportClient(): Super(FObjectInitializer::Get()) {};

	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance,
	                  bool bCreateNewAudioDevice = true) override;
};
