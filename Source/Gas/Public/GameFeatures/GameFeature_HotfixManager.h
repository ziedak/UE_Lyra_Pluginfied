// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureStateChangeObserver.h"
#include "UObject/Object.h"
#include "GameFeature_HotfixManager.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGameFeature_HotfixManager final : public UObject, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	virtual void OnGameFeatureLoading(const UGameFeatureData* GameFeatureData, const FString& PluginURL) override;
};
