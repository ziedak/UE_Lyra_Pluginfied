// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "CheatDeveloperSettings.generated.h"

struct FCheatToRun;
/**
 * Developer settings / editor cheats
 */
UCLASS(config=EditorPerProjectUserSettings, MinimalAPI)
class UCheatDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	// List of cheats to auto-run during 'play in editor'
	UPROPERTY(config, EditAnywhere, Category=Cheats)
	TArray<FCheatToRun> CheatsToRun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config, Category=Bots)
	bool bAllowPlayerBotsToAttack = true;
};
