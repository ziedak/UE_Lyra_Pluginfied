// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "LyraAimSensitivityData.generated.h"

enum class ELyraGamepadSensitivity : uint8;

class UObject;

UENUM(BlueprintType)
enum class ELyraGamepadSensitivity : uint8
{
	Invalid = 0 UMETA(Hidden),

	Slow UMETA(DisplayName = "01 - Slow"),
	SlowPlus UMETA(DisplayName = "02 - Slow+"),
	SlowPlusPlus UMETA(DisplayName = "03 - Slow++"),
	Normal UMETA(DisplayName = "04 - Normal"),
	NormalPlus UMETA(DisplayName = "05 - Normal+"),
	NormalPlusPlus UMETA(DisplayName = "06 - Normal++"),
	Fast UMETA(DisplayName = "07 - Fast"),
	FastPlus UMETA(DisplayName = "08 - Fast+"),
	FastPlusPlus UMETA(DisplayName = "09 - Fast++"),
	Insane UMETA(DisplayName = "10 - Insane"),

	MAX UMETA(Hidden),
};

/** Defines a set of gamepad sensitivity to a float value. */
UCLASS(BlueprintType, Const,
	Meta = (DisplayName = "Lyra Aim Sensitivity Data", ShortTooltip =
		"Data asset used to define a map of Gamepad Sensitivty to a float value."))
class INPUTCONFIG_API ULyraAimSensitivityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULyraAimSensitivityData(const FObjectInitializer& ObjectInitializer);

	float SensitivityEnumToFloat(const ELyraGamepadSensitivity InSensitivity) const;

protected:
	/** Map of SensitivityMap settings to their corresponding float */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ELyraGamepadSensitivity, float> SensitivityMap;
};
