// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureStateChangeObserver.h"
#include "UObject/Object.h"
#include "GameFeature_AddGameplayCuePaths.generated.h"

/**
 *  //
 */
UCLASS()
class GAS_API UGameFeature_AddGameplayCuePaths : public UObject, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName,
	                                      const FString& PluginURL) override;
	virtual void OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName,
	                                        const FString& PluginURL) override;

};
