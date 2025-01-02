// Copyright Epic Games, Inc. All Rights Reserved.

#include "Basic/MaterialProgressBar.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MaterialProgressBar)

void UMaterialProgressBar::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (!Image_Bar) { return; }

	Image_Bar->SetBrushFromMaterial(bUseStroke ? StrokeMaterial : NoStrokeMaterial);
	CachedMID = nullptr;
	CachedProgress = -1.0f;
	CachedStartProgress = -1.0f;

#if WITH_EDITORONLY_DATA
	if (IsDesignTime()) { SetProgress_Internal(DesignTime_Progress); }
#endif

	if (UMaterialInstanceDynamic* MID = GetBarDynamicMaterial())
	{
		if (bOverrideDefaultSegmentEdge) { MID->SetScalarParameterValue(TEXT("SegmentEdge"), SegmentEdge); }
		if (bOverrideDefaultSegments) { MID->SetScalarParameterValue(TEXT("Segments"), static_cast<float>(Segments)); }

		if (bOverrideDefaultFillEdgeSoftness)
		{
			MID->SetScalarParameterValue(TEXT("FillEdgeSoftness"), FillEdgeSoftness);
		}

		if (bOverrideDefaultGlowEdge) { MID->SetScalarParameterValue(TEXT("GlowEdge"), GlowEdge); }
		if (bOverrideDefaultGlowSoftness) { MID->SetScalarParameterValue(TEXT("GlowSoftness"), GlowSoftness); }
		if (bOverrideDefaultOutlineScale) { MID->SetScalarParameterValue(TEXT("OutlineScale"), OutlineScale); }
	}

	if (bOverrideDefaultColorA) { SetColorA_Internal(CachedColorA); }
	if (bOverrideDefaultColorB) { SetColorB_Internal(CachedColorB); }
	if (bOverrideDefaultColorBackground) { SetColorBackground_Internal(CachedColorBackground); }
}

#if WITH_EDITOR
void UMaterialProgressBar::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	if (!IsDesignTime() || !Image_Bar) { return; }

	const auto Mid = GetBarDynamicMaterial();
	if (!Mid) { return; }

	if (!bOverrideDefaultColorA) { Mid->GetVectorParameterValue(TEXT("ColorA"), CachedColorA); }
	if (!bOverrideDefaultColorB) { Mid->GetVectorParameterValue(TEXT("ColorB"), CachedColorB); }

	if (!bOverrideDefaultColorBackground)
	{
		Mid->GetVectorParameterValue(TEXT("Unfilled Color"), CachedColorBackground);
	}

	if (!bOverrideDefaultSegmentEdge) { Mid->GetScalarParameterValue(TEXT("SegmentEdge"), SegmentEdge); }

	if (!bOverrideDefaultSegments)
	{
		float SegmentsFloat;
		Mid->GetScalarParameterValue(TEXT("Segments"), SegmentsFloat);
		Segments = FMath::TruncToInt(SegmentsFloat);
	}

	if (!bOverrideDefaultFillEdgeSoftness) { Mid->GetScalarParameterValue(TEXT("FillEdgeSoftness"), FillEdgeSoftness); }
	if (!bOverrideDefaultGlowEdge) { Mid->GetScalarParameterValue(TEXT("GlowEdge"), GlowEdge); }
	if (!bOverrideDefaultGlowSoftness) { Mid->GetScalarParameterValue(TEXT("GlowSoftness"), GlowSoftness); }
	if (!bOverrideDefaultOutlineScale) { Mid->GetScalarParameterValue(TEXT("OutlineScale"), OutlineScale); }
}
#endif

void UMaterialProgressBar::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (BoundAnim_FillBar == Animation) { OnFillAnimationFinished.Broadcast(); }
}

void UMaterialProgressBar::SetProgress(const float Progress)
{
	if (CachedProgress != Progress) { SetProgress_Internal(Progress); }
}

void UMaterialProgressBar::SetStartProgress(const float StartProgress)
{
	if (CachedStartProgress != StartProgress) { SetStartProgress_Internal(StartProgress); }
}

void UMaterialProgressBar::SetColorA(const FLinearColor ColorA)
{
	if (CachedColorA != ColorA) { SetColorA_Internal(ColorA); }
}

void UMaterialProgressBar::SetColorB(const FLinearColor ColorB)
{
	if (CachedColorB != ColorB) { SetColorB_Internal(ColorB); }
}

void UMaterialProgressBar::SetColorBackground(const FLinearColor ColorBackground)
{
	if (CachedColorBackground != ColorBackground) { SetColorBackground_Internal(ColorBackground); }
}

void UMaterialProgressBar::AnimateProgressFromStart(const float Start, const float End, const float AnimSpeed)
{
	SetStartProgress(Start);
	SetProgress(End);
	PlayAnimation(BoundAnim_FillBar, 0.0f, 1, EUMGSequencePlayMode::Forward, AnimSpeed);
}

void UMaterialProgressBar::AnimateProgressFromCurrent(const float End, const float AnimSpeed)
{
	const auto Mid = GetBarDynamicMaterial();
	if (!Mid) { return; }

	const float CurrentStart = Mid->K2_GetScalarParameterValue(TEXT("StartProgress"));
	const float CurrentEnd = Mid->K2_GetScalarParameterValue(TEXT("Progress"));
	const float CurrentFill = Mid->K2_GetScalarParameterValue(TEXT("FillAmount"));
	const float NewStart = FMath::Lerp(CurrentStart, CurrentEnd, CurrentFill);
	AnimateProgressFromStart(NewStart, End, AnimSpeed);
}

void UMaterialProgressBar::SetProgress_Internal(const float Progress)
{
	const auto Mid = GetBarDynamicMaterial();
	if (!Mid) { return; }

	CachedProgress = Progress;
	Mid->SetScalarParameterValue(TEXT("Progress"), CachedProgress);
}

void UMaterialProgressBar::SetStartProgress_Internal(const float StartProgress)
{
	const auto Mid = GetBarDynamicMaterial();
	if (!Mid) { return; }

	CachedStartProgress = StartProgress;
	Mid->SetScalarParameterValue(TEXT("StartProgress"), CachedStartProgress);
}

void UMaterialProgressBar::SetColorA_Internal(const FLinearColor ColorA)
{
	const auto Mid = GetBarDynamicMaterial();
	if (!Mid) { return; }

	CachedColorA = ColorA;
	Mid->SetVectorParameterValue(TEXT("ColorA"), CachedColorA);
}

void UMaterialProgressBar::SetColorB_Internal(const FLinearColor ColorB)
{
	const auto Mid = GetBarDynamicMaterial();
	if (!Mid) { return; }

	CachedColorB = ColorB;
	Mid->SetVectorParameterValue(TEXT("ColorB"), CachedColorB);
}

void UMaterialProgressBar::SetColorBackground_Internal(FLinearColor ColorBackground)
{
	const auto Mid = GetBarDynamicMaterial();
	if (!Mid) { return; }

	CachedColorBackground = ColorBackground;
	Mid->SetVectorParameterValue(TEXT("Unfilled Color"), CachedColorBackground);
}

UMaterialInstanceDynamic* UMaterialProgressBar::GetBarDynamicMaterial() const
{
	if (!CachedMID) { CachedMID = Image_Bar->GetDynamicMaterial(); }
	return CachedMID;
}
