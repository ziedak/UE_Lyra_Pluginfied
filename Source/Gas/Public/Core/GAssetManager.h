// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetManager/BaseAssetManager.h"
#include "GAssetManager.generated.h"

class UGasGameData;
class UGasPawnData;
/**
 * 
 */
UCLASS(Config = Game)
class GAS_API UGAssetManager : public UBaseAssetManager
{
	GENERATED_BODY()

public:
	// Returns the AssetManager singleton object.
	static UGAssetManager& Get();

protected:
	// Global game data asset to use (DefaultGame.ini).
	UPROPERTY(Config)
	TSoftObjectPtr<UGasGameData> BaseGameDataPath;

	// Pawn data used when spawning player pawns if there isn't one set on the player state (DefaultGame.ini).
	UPROPERTY(Config)
	TSoftObjectPtr<UGasPawnData> DefaultPawnData;

#pragma region UAssetManager interface
	virtual void StartInitialLoading() override;
#if WITH_EDITOR
	virtual void PreBeginPIE(bool bStartSimulate) override;
#endif

#pragma endregion


	// Sets up the ability system
	void InitializeGameplayCueManager() const;

public:
	const UGasGameData& GetGameData();

	UGasPawnData* GetDefaultPawnData() const;
};
