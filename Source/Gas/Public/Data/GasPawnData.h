// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraInputConfig.h"
#include "Engine/DataAsset.h"
#include "GasPawnData.generated.h"

class APawn;
class UBaseAbilitySet;
class UBaseAbilityTagRelationshipMapping;
class UBaseInputConfig;
class UObject;

/**
 * UGasPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Gas Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class GAS_API UGasPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UGasPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from AGasPawn or AGasCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gas|Pawn")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gas|Abilities")
	TArray<TObjectPtr<UBaseAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gas|Abilities")
	TObjectPtr<UBaseAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gas|Input")
	const ULyraInputConfig* InputConfig;

	//// Default camera mode used by player controlled pawns.
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gas|Camera")
	//TSubclassOf<UGasCameraMode> DefaultCameraMode;
};
