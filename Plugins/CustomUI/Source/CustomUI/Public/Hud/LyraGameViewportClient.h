// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameViewportClient.h"

#include "LyraGameViewportClient.generated.h"

class UGameInstance;
class UObject;

UCLASS(Abstract, BlueprintType)
class CUSTOMUI_API ULyraGameViewportClient : public UCommonGameViewportClient
{
	GENERATED_BODY()

public:
	//ULyraGameViewportClient();
	ULyraGameViewportClient(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()): Super(
		ObjectInitializer) {}

	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance,
	                  bool bCreateNewAudioDevice = true) override;
};
