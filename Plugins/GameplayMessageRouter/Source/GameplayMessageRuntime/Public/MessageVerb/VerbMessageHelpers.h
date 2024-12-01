// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "VerbMessageHelpers.generated.h"

class UObject;
struct FFrame;
struct FGameplayCueParameters;
struct FVerbMessage;
class APlayerController;
class APlayerState;
/**
 *	Helper functions for converting between VerbMessages and GameplayCueParameters.	
 *	
 */
UCLASS()
class GAMEPLAYMESSAGERUNTIME_API UVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Helpers")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);
	
	UFUNCTION(BlueprintCallable, Category = "Helpers")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Helpers")
	static FGameplayCueParameters VerbMessageToCueParameters(const FVerbMessage& Message);

	UFUNCTION(BlueprintCallable, Category = "Helpers")
	static FVerbMessage CueParametersToVerbMessage(const FGameplayCueParameters& Params);
};


