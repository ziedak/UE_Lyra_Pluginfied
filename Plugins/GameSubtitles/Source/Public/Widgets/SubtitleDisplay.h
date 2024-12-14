// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "Styling/SlateTypes.h"
#include "SubtitleDisplaySubsystem.h"

#include "SubtitleDisplay.generated.h"

class USubtitleDisplayOptions;

struct FSubtitleFormat;

UCLASS(BlueprintType, Blueprintable, meta = (DisableNativeTick))
class GAMESUBTITLES_API USubtitleDisplay : public UWidget
{
	GENERATED_UCLASS_BODY()
	UPROPERTY(EditAnywhere, Category = "Display Info")
	FSubtitleFormat Format;

	UPROPERTY(EditAnywhere, Category = "Display Info")
	TObjectPtr<USubtitleDisplayOptions> Options;

	// Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs.
	UPROPERTY(EditAnywhere, Category="Display Info")
	float WrapTextAt;

	UFUNCTION(BlueprintCallable, Category = Subtitles,
		Meta = (Tooltip = "True if there are subtitles currently.  False if the subtitle text is empty."))
	bool HasSubtitles() const;

	/** Preview text to be displayed when designing the widget */
	UPROPERTY(EditAnywhere, Category="Preview")
	bool bPreviewMode;

	/** Preview text to be displayed when designing the widget */
	UPROPERTY(EditAnywhere, Category="Preview")
	FText PreviewText;

	// UWidget Public Interface
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const override;
#endif
	// End UWidget Public Interface

protected:
	// UWidget Protected Interface
	virtual TSharedRef<class SWidget> RebuildWidget() override;
	// End UWidget Protected Interface

	void HandleSubtitleDisplayOptionsChanged(const FSubtitleFormat& InDisplayFormat);

private:
	void RebuildStyle();

	UPROPERTY(Transient)
	FTextBlockStyle GeneratedStyle;

	UPROPERTY(Transient)
	FSlateBrush GeneratedBackgroundBorder;

	/** The actual widget for displaying subtitle data */
	TSharedPtr<class SSubtitleDisplay> SubtitleWidget;
};
