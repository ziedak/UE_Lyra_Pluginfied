// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ExperienceDefinition_DA.generated.h"

class UGameFeatureAction;
class UExperienceActionSet_DA;
/**
 * Definition of an experience
 */
UCLASS(BlueprintType, Const)
class CUSTOMCORE_API UExperienceDefinition_DA : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnableList;

	/** The default pawn class to spawn for players */
	//@TODO: Make soft?
	// use generic type for pawn data PawnData : public UPrimaryDataAsset
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const UPrimaryDataAsset> DefaultPawnData;

	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> GameFeatureActions;

	// List of additional action sets to compose into this experience
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<UExperienceActionSet_DA>> ExperienceActionSets;


	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
};
