// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"

#include "SubtitleDisplayOptions.generated.h"

UENUM()
enum class ESubtitleDisplayTextSize : uint8
{
	ExtraSmall,
	Small,
	Medium,
	Large,
	ExtraLarge,
	TextSize_MAX
};

UENUM()
enum class ESubtitleDisplayTextColor : uint8
{
	White,
	Black,
	Red,
	Green,
	Blue,
	Yellow,
	Cyan,
	Magenta,
	TextColor_MAX
};

UENUM()
enum class ESubtitleDisplayTextBorder : uint8
{
	None,
	Outline,
	DropShadow,
	TextBorder_MAX
};

UENUM()
enum class ESubtitleDisplayBackgroundOpacity : uint8
{
	Clear,
	Low,
	Medium,
	High,
	Solid,
	BackgroundOpacity_MAX
};

/**
 * 
 */
UCLASS(BlueprintType)
class GAMESUBTITLES_API USubtitleDisplayOptions : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	FSlateFontInfo Font;

	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	int32 DisplayTextSizes[static_cast<int32>(ESubtitleDisplayTextSize::TextSize_MAX)];

	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	FLinearColor DisplayTextColors[static_cast<int32>(ESubtitleDisplayTextColor::TextColor_MAX)];

	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	float DisplayBorderSize[static_cast<int32>(ESubtitleDisplayTextBorder::TextBorder_MAX)];

	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	float DisplayBackgroundOpacity[static_cast<int32>(ESubtitleDisplayBackgroundOpacity::BackgroundOpacity_MAX)];

	UPROPERTY(EditDefaultsOnly, Category = "Display Info")
	FSlateBrush BackgroundBrush;
};
