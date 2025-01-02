// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CommonGameSettings.generated.h"

class UGameUIPolicy;
/**
 * Settings for a loading screen system.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="Common Game"))
class UCommonGameSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	// The widget to load for the loading screen.
	// UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/GameUIPolicy"))
	// FSoftClassPath DefaultUIPolicyClass;

	UPROPERTY(config, EditAnywhere, Category=Display, meta=(MetaClass="/Script/GameUIPolicy"))
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;
};