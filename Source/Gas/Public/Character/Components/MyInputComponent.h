// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraInputComponent.h"
#include "MyInputComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UMyInputComponent : public ULyraInputComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMyInputComponent(const FObjectInitializer& ObjectInitializer);
};
