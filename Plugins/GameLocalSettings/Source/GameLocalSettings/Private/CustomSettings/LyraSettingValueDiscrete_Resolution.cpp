// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSettings/LyraSettingValueDiscrete_Resolution.h"

#include "Framework/Application/SlateApplication.h"
#include "GameFramework/GameUserSettings.h"
#include "RHI.h"
#include "UnrealEngine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSettingValueDiscrete_Resolution)

#define LOCTEXT_NAMESPACE "LyraSettings"

ULyraSettingValueDiscrete_Resolution::ULyraSettingValueDiscrete_Resolution()
{
}

void ULyraSettingValueDiscrete_Resolution::OnInitialized()
{
	Super::OnInitialized();

	InitializeResolutions();
}

void ULyraSettingValueDiscrete_Resolution::StoreInitial()
{
	// Ignored
}

void ULyraSettingValueDiscrete_Resolution::ResetToDefault()
{
	// Ignored
}

void ULyraSettingValueDiscrete_Resolution::RestoreToInitial()
{
	// Ignored
}

void ULyraSettingValueDiscrete_Resolution::SetDiscreteOptionByIndex(int32 Index)
{
	if (!Resolutions.IsValidIndex(Index) || !Resolutions[Index].IsValid()) return;

	GEngine->GetGameUserSettings()->SetScreenResolution(Resolutions[Index]->GetResolution());
	NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 ULyraSettingValueDiscrete_Resolution::GetDiscreteOptionIndex() const
{
	const UGameUserSettings* UserSettings = CastChecked<const UGameUserSettings>(GEngine->GetGameUserSettings());
	return FindIndexOfDisplayResolutionForceValid(UserSettings->GetScreenResolution());
}

TArray<FText> ULyraSettingValueDiscrete_Resolution::GetDiscreteOptions() const
{
	TArray<FText> ReturnResolutionTexts;
	for (int32 i = 0; i < Resolutions.Num(); ++i)
	{
		ReturnResolutionTexts.Add(Resolutions[i]->GetDisplayText());
	}

	return ReturnResolutionTexts;
}

void ULyraSettingValueDiscrete_Resolution::OnDependencyChanged()
{
	const FIntPoint CurrentResolution = GEngine->GetGameUserSettings()->GetScreenResolution();
	SelectAppropriateResolutions();
	SetDiscreteOptionByIndex(FindClosestResolutionIndex(CurrentResolution));
}


void ULyraSettingValueDiscrete_Resolution::InitializeResolutions()
{
	ResetResolution();

	FDisplayMetrics InitialDisplayMetrics;
	FSlateApplication::Get().GetInitialDisplayMetrics(InitialDisplayMetrics);

	FScreenResolutionArray ScreenResolutions;
	RHIGetAvailableResolutions(ScreenResolutions, true);

	InitializeWindowedResolutions(InitialDisplayMetrics);
	InitializeWindowedFullscreenResolutions(InitialDisplayMetrics, ScreenResolutions);
	InitializeFullscreenResolutions(ScreenResolutions);

	SelectAppropriateResolutions();
}

void ULyraSettingValueDiscrete_Resolution::ResetResolution()
{
	Resolutions.Empty();
	ResolutionsFullscreen.Empty();
	ResolutionsWindowed.Empty();
	ResolutionsWindowedFullscreen.Empty();
}

void ULyraSettingValueDiscrete_Resolution::InitializeWindowedResolutions(const FDisplayMetrics& InitialDisplayMetrics)
{
	TArray<FIntPoint> WindowedResolutions;
	const FIntPoint MinResolution(1280, 720);
	// Use the primary display resolution minus 1 to exclude the primary display resolution from the list.
	// This is so you don't make a window so large that part of the game is off screen and you are unable to change resolutions back.
	const FIntPoint MaxResolution(InitialDisplayMetrics.PrimaryDisplayWidth - 1,
	                              InitialDisplayMetrics.PrimaryDisplayHeight - 1);

	// Excluding 4:3 and below
	constexpr float MinAspectRatio = 16 / 10.f;

	if (MaxResolution.X >= MinResolution.X &&
		MaxResolution.Y >= MinResolution.Y)
	{
		GetStandardWindowResolutions(MinResolution, MaxResolution, MinAspectRatio, WindowedResolutions);
	}

	if (GSystemResolution.WindowMode == EWindowMode::Windowed)
	{
		WindowedResolutions.AddUnique(FIntPoint(GSystemResolution.ResX, GSystemResolution.ResY));
		WindowedResolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
		{
			return A.X != B.X ? A.X < B.X : A.Y < B.Y;
		});
	}

	// If no resolutions were found, add the primary display resolution
	// If there were no standard resolutions. Add the primary display size, just so one exists.
	// This might happen if we are running on a non-standard device.
	if (WindowedResolutions.Num() == 0)
	{
		WindowedResolutions.Add(FIntPoint(InitialDisplayMetrics.PrimaryDisplayWidth,
		                                  InitialDisplayMetrics.PrimaryDisplayHeight));
	}

	ResolutionsWindowed.Empty(WindowedResolutions.Num());
	for (const FIntPoint& Res : WindowedResolutions)
	{
		TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
		Entry->Width = Res.X;
		Entry->Height = Res.Y;

		ResolutionsWindowed.Add(Entry);
	}
}

void ULyraSettingValueDiscrete_Resolution::InitializeWindowedFullscreenResolutions(
	const FDisplayMetrics& InitialDisplayMetrics, const FScreenResolutionArray& ScreenResolutions)
{
	const FScreenResolutionRHI* RHIInitialResolution = ScreenResolutions.FindByPredicate(
		[InitialDisplayMetrics](const FScreenResolutionRHI& ScreenRes)
		{
			return ScreenRes.Width == InitialDisplayMetrics.PrimaryDisplayWidth && ScreenRes.Height ==
				InitialDisplayMetrics.PrimaryDisplayHeight;
		});

	const TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
	if (RHIInitialResolution)
	{
		// If this is in the official list use that
		Entry->Width = RHIInitialResolution->Width;
		Entry->Height = RHIInitialResolution->Height;
		Entry->RefreshRate = RHIInitialResolution->RefreshRate;
	}
	else
	{
		// Custom resolution the RHI doesn't expect
		Entry->Width = InitialDisplayMetrics.PrimaryDisplayWidth;
		Entry->Height = InitialDisplayMetrics.PrimaryDisplayHeight;
		// TODO: Unsure how to calculate refresh rate for custom resolutions
		Entry->RefreshRate = FPlatformMisc::GetMaxRefreshRate();
	}

	ResolutionsWindowedFullscreen.Add(Entry);
}

void ULyraSettingValueDiscrete_Resolution::InitializeFullscreenResolutions(
	const FScreenResolutionArray& ScreenResolutions)
{
	// Determine available full-screen modes and filter out any that are not supported by the current platform
	if (ScreenResolutions.Num() <= 0) return;

	// try more strict first then more relaxed, we want at least one resolution to remain
	for (int32 FilterThreshold = 0; FilterThreshold < 3; ++FilterThreshold)
	{
		for (int32 ModeIndex = 0; ModeIndex < ScreenResolutions.Num(); ModeIndex++)
		{
			const FScreenResolutionRHI& ScreenRes = ScreenResolutions[ModeIndex];

			if (ShouldAllowFullScreenResolution(ScreenRes, FilterThreshold))
			{
				TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
				Entry->Width = ScreenRes.Width;
				Entry->Height = ScreenRes.Height;
				Entry->RefreshRate = ScreenRes.RefreshRate;

				ResolutionsFullscreen.Add(Entry);
			}
		}
		// we found some resolutions, otherwise we try with more relaxed tests
		if (ResolutionsFullscreen.Num()) break;
	}
}

void ULyraSettingValueDiscrete_Resolution::SelectAppropriateResolutions()
{
	const EWindowMode::Type WindowMode = GEngine->GetGameUserSettings()->GetFullscreenMode();
	if (LastWindowMode == WindowMode) return;

	LastWindowMode = WindowMode;
	Resolutions.Empty();

	switch (WindowMode)
	{
	case EWindowMode::Windowed:
		Resolutions.Append(ResolutionsWindowed);
		break;
	case EWindowMode::WindowedFullscreen:
		Resolutions.Append(ResolutionsWindowedFullscreen);
		break;
	case EWindowMode::Fullscreen:
		Resolutions.Append(ResolutionsFullscreen);
		break;
	case EWindowMode::NumWindowModes:
	default: break;
	}

	NotifyEditConditionsChanged();
}

// To filter out odd resolution so UI and testing has less issues. This is game specific.
// @param ScreenRes resolution and
// @param FilterThreshold 0/1/2 to make sure we get at least some resolutions (might be an issues with UI but at least we get some resolution entries)
bool ULyraSettingValueDiscrete_Resolution::ShouldAllowFullScreenResolution(
	const FScreenResolutionRHI& SrcScreenRes, const int32 FilterThreshold) const
{
	FScreenResolutionRHI ScreenRes = SrcScreenRes;

	// expected: 4:3=1.333, 16:9=1.777, 16:10=1.6, multi-monitor-wide: >2
	bool bIsPortrait = ScreenRes.Width < ScreenRes.Height;
	float AspectRatio = static_cast<float>(ScreenRes.Width) / static_cast<float>(ScreenRes.Height);

	// If portrait, flip values back to landscape so we can don't have to special case all the tests below
	if (bIsPortrait)
	{
		AspectRatio = 1.0f / AspectRatio;
		ScreenRes.Width = SrcScreenRes.Height;
		ScreenRes.Height = SrcScreenRes.Width;
	}

	return
		IsAspectRatioValid(AspectRatio, FilterThreshold) && IsResolutionSizeValid(ScreenRes, FilterThreshold);
}

bool ULyraSettingValueDiscrete_Resolution::IsAspectRatioValid(const float AspectRatio,
                                                              const int32 FilterThreshold) const
{
	if (FilterThreshold >= 1) return true;

	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetInitialDisplayMetrics(DisplayMetrics);

	// Default display aspect to required aspect in case this platform can't provide the information. Forces acceptance of this resolution.
	float DisplayAspect = AspectRatio;

	for (const FMonitorInfo& MonitorInfo : DisplayMetrics.MonitorInfo)
	{
		if (MonitorInfo.bIsPrimary)
		{
			DisplayAspect = static_cast<float>(MonitorInfo.NativeWidth) / static_cast<float>(MonitorInfo.NativeHeight);
			break;
		}
	}

	return FMath::Abs(DisplayAspect - AspectRatio) <= KINDA_SMALL_NUMBER;
}

bool ULyraSettingValueDiscrete_Resolution::IsResolutionSizeValid(const FScreenResolutionRHI& ScreenRes,
                                                                 const int32 FilterThreshold) const
{
	if (FilterThreshold >= 2)return true;
	return ScreenRes.Width >= 1280 && ScreenRes.Height >= 720;
}

int32 ULyraSettingValueDiscrete_Resolution::FindIndexOfDisplayResolution(const FIntPoint& InPoint) const
{
	// find the current res
	for (int32 i = 0, Num = Resolutions.Num(); i < Num; ++i)
	{
		if (Resolutions[i]->GetResolution() == InPoint)
			return i;
	}

	return INDEX_NONE;
}

int32 ULyraSettingValueDiscrete_Resolution::FindIndexOfDisplayResolutionForceValid(const FIntPoint& InPoint) const
{
	const int32 Result = FindIndexOfDisplayResolution(InPoint);
	return (Result == INDEX_NONE && Resolutions.Num() > 0) ? Resolutions.Num() - 1 : Result;
}

int32 ULyraSettingValueDiscrete_Resolution::FindClosestResolutionIndex(const FIntPoint& Resolution) const
{
	int32 Index = 0;
	int32 LastDiff = Resolution.SizeSquared();

	for (int32 i = 0, Num = Resolutions.Num(); i < Num; ++i)
	{
		// We compare the squared diagonals
		const int32 Diff = FMath::Abs(Resolution.SizeSquared() - Resolutions[i]->GetResolution().SizeSquared());
		if (Diff <= LastDiff)
			Index = i;

		LastDiff = Diff;
	}

	return Index;
}

void ULyraSettingValueDiscrete_Resolution::GetStandardWindowResolutions(const FIntPoint& MinResolution,
                                                                        const FIntPoint& MaxResolution,
                                                                        const float MinAspectRatio,
                                                                        TArray<FIntPoint>& OutResolutions)
{
	// Standard resolutions as provided by Wikipedia (http://en.wikipedia.org/wiki/Graphics_display_resolution)
	static TArray<FIntPoint> StandardResolutions;
	if (StandardResolutions.Num() == 0)
	{
		// Extended Graphics Array
		{
			new(StandardResolutions) FIntPoint(1024, 768); // XGA

			// WXGA (3 versions)
			new(StandardResolutions) FIntPoint(1366, 768); // FWXGA
			new(StandardResolutions) FIntPoint(1360, 768);
			new(StandardResolutions) FIntPoint(1280, 800);

			new(StandardResolutions) FIntPoint(1152, 864); // XGA+
			new(StandardResolutions) FIntPoint(1440, 900); // WXGA+
			new(StandardResolutions) FIntPoint(1280, 1024); // SXGA
			new(StandardResolutions) FIntPoint(1400, 1050); // SXGA+
			new(StandardResolutions) FIntPoint(1680, 1050); // WSXGA+
			new(StandardResolutions) FIntPoint(1600, 1200); // UXGA
			new(StandardResolutions) FIntPoint(1920, 1200); // WUXGA
		}

		// Quad Extended Graphics Array
		{
			new(StandardResolutions) FIntPoint(2048, 1152); // QWXGA
			new(StandardResolutions) FIntPoint(2048, 1536); // QXGA
			new(StandardResolutions) FIntPoint(2560, 1600); // WQXGA
			new(StandardResolutions) FIntPoint(2560, 2048); // QSXGA
			new(StandardResolutions) FIntPoint(3200, 2048); // WQSXGA
			new(StandardResolutions) FIntPoint(3200, 2400); // QUXGA
			new(StandardResolutions) FIntPoint(3840, 2400); // WQUXGA
		}

		// Hyper Extended Graphics Array
		{
			new(StandardResolutions) FIntPoint(4096, 3072); // HXGA
			new(StandardResolutions) FIntPoint(5120, 3200); // WHXGA
			new(StandardResolutions) FIntPoint(5120, 4096); // HSXGA
			new(StandardResolutions) FIntPoint(6400, 4096); // WHSXGA
			new(StandardResolutions) FIntPoint(6400, 4800); // HUXGA
			new(StandardResolutions) FIntPoint(7680, 4800); // WHUXGA
		}

		// High-Definition
		{
			new(StandardResolutions) FIntPoint(640, 360); // nHD
			new(StandardResolutions) FIntPoint(960, 540); // qHD
			new(StandardResolutions) FIntPoint(1280, 720); // HD
			new(StandardResolutions) FIntPoint(1920, 1080); // FHD
			new(StandardResolutions) FIntPoint(2560, 1440); // QHD
			new(StandardResolutions) FIntPoint(3200, 1800); // WQXGA+
			new(StandardResolutions) FIntPoint(3840, 2160); // UHD 4K
			new(StandardResolutions) FIntPoint(4096, 2160); // Digital Cinema Initiatives 4K
			new(StandardResolutions) FIntPoint(7680, 4320); // FUHD
			new(StandardResolutions) FIntPoint(5120, 2160); // UHD 5K
			new(StandardResolutions) FIntPoint(5120, 2880); // UHD+
			new(StandardResolutions) FIntPoint(15360, 8640); // QUHD
		}

		// Sort the list by total resolution size
		StandardResolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
		{
			return (A.X * A.Y) < (B.X * B.Y);
		});
	}

	// Return all standard resolutions that are within the size constraints
	for (const auto& Resolution : StandardResolutions)
	{
		if (Resolution.X >= MinResolution.X && Resolution.Y >= MinResolution.Y &&
			Resolution.X <= MaxResolution.X && Resolution.Y <= MaxResolution.Y)
		{
			const float AspectRatio = Resolution.X / static_cast<float>(Resolution.Y);
			if (AspectRatio > MinAspectRatio || FMath::IsNearlyEqual(AspectRatio, MinAspectRatio))
			{
				OutResolutions.Add(Resolution);
			}
		}
	}
}

FText ULyraSettingValueDiscrete_Resolution::FScreenResolutionEntry::GetDisplayText() const
{
	if (!OverrideText.IsEmpty())
		return OverrideText;

	FText Aspect = FText::GetEmpty();

	// expected: 4:3=1.333, 16:9=1.777, 16:10=1.6, multi-monitor-wide: >2
	const float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);

	if (FMath::Abs(AspectRatio - (4.0f / 3.0f)) < KINDA_SMALL_NUMBER)
		Aspect = LOCTEXT("AspectRatio-4:3", "4:3");
	else if (FMath::Abs(AspectRatio - (16.0f / 9.0f)) < KINDA_SMALL_NUMBER)
		Aspect = LOCTEXT("AspectRatio-16:9", "16:9");
	else if (FMath::Abs(AspectRatio - (16.0f / 10.0f)) < KINDA_SMALL_NUMBER)
		Aspect = LOCTEXT("AspectRatio-16:10", "16:10");
	else if (FMath::Abs(AspectRatio - (3.0f / 4.0f)) < KINDA_SMALL_NUMBER)
		Aspect = LOCTEXT("AspectRatio-3:4", "3:4");
	else if (FMath::Abs(AspectRatio - (9.0f / 16.0f)) < KINDA_SMALL_NUMBER)
		Aspect = LOCTEXT("AspectRatio-9:16", "9:16");
	else if (FMath::Abs(AspectRatio - (10.0f / 16.0f)) < KINDA_SMALL_NUMBER)
		Aspect = LOCTEXT("AspectRatio-10:16", "10:16");

	FNumberFormattingOptions Options;
	Options.UseGrouping = false;

	FFormatNamedArguments Args;
	Args.Add(TEXT("X"), FText::AsNumber(Width, &Options));
	Args.Add(TEXT("Y"), FText::AsNumber(Height, &Options));
	Args.Add(TEXT("AspectRatio"), Aspect);
	Args.Add(TEXT("RefreshRate"), RefreshRate);

	return FText::Format(LOCTEXT("AspectRatio", "{X} x {Y}"), Args);
}

#undef LOCTEXT_NAMESPACE
