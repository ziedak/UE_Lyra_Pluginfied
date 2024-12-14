// Copyright Epic Games, Inc. All Rights Reserved.

#include "Settings/LyraSettingsLocal.h"
#include "Audio/LyraAudioMixEffectsSubsystem.h"
#include "Audio/LyraAudioSettings.h"
#include "AudioModulationStatics.h"
#include "CommonInputSubsystem.h"
#include "CommonUISettings.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformFramePacer.h"
#include "HAL/PlatformFramePacer.h"
#include "ICommonUIModule.h"
#include "Misc/App.h"
#include "Performance/LyraPerformanceSettings.h"
#include "Performance/LyraPerformanceStatTypes.h"
#include "Performance/PlatformEmulationSettings.h"
#include "SoundControlBus.h"
#include "SoundControlBusMix.h"
#include "Widgets/Layout/SSafeZone.h"
// #include "GenericPlatform/GenericPlatformProcess.h"
// #include "RHI.h"
// #include "EnhancedActionKeyMapping.h"
// #include "EnhancedInputSubsystems.h"
// #include "PlayerMappableInputConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSettingsLocal)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_BinauralSettingControlledByOS,
                              "Platform.Trait.BinauralSettingControlledByOS");

//////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
static TAutoConsoleVariable<bool> CVarApplyFrameRateSettingsInPIE(TEXT("Lyra.Settings.ApplyFrameRateSettingsInPIE"),
                                                                  false,
                                                                  TEXT("Should we apply frame rate settings in PIE?"),
                                                                  ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyFrontEndPerformanceOptionsInPIE(
	TEXT("Lyra.Settings.ApplyFrontEndPerformanceOptionsInPIE"),
	false,
	TEXT("Do we apply front-end specific performance options in PIE?"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyDeviceProfilesInPIE(TEXT("Lyra.Settings.ApplyDeviceProfilesInPIE"),
                                                               false,
                                                               TEXT(
	                                                               "Should we apply experience/platform emulated device profiles in PIE?"),
                                                               ECVF_Default);
#endif

//////////////////////////////////////////////////////////////////////
// Console frame pacing

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenTargetFps(
	TEXT("Lyra.DeviceProfile.Console.TargetFPS"),
	-1,
	TEXT("Target FPS when being driven by device profile"),
	ECVF_Default | ECVF_Preview);

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenFrameSyncType(
	TEXT("Lyra.DeviceProfile.Console.FrameSyncType"),
	-1,
	TEXT("Sync type when being driven by device profile. Corresponds to r.GTSyncType"),
	ECVF_Default | ECVF_Preview);

//////////////////////////////////////////////////////////////////////
// Mobile frame pacing

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenMobileDefaultFrameRate(
	TEXT("Lyra.DeviceProfile.Mobile.DefaultFrameRate"),
	30,
	TEXT("Default FPS when being driven by device profile"),
	ECVF_Default | ECVF_Preview);

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenMobileMaxFrameRate(
	TEXT("Lyra.DeviceProfile.Mobile.MaxFrameRate"),
	30,
	TEXT("Max FPS when being driven by device profile"),
	ECVF_Default | ECVF_Preview);

//////////////////////////////////////////////////////////////////////

static TAutoConsoleVariable<FString> CVarMobileQualityLimits(
	TEXT("Lyra.DeviceProfile.Mobile.OverallQualityLimits"),
	TEXT(""),
	TEXT(
		"List of limits on resolution quality of the form \"FPS:MaxQuality,FPS2:MaxQuality2,...\", kicking in when FPS is at or above the threshold"),
	ECVF_Default | ECVF_Preview);

static TAutoConsoleVariable<FString> CVarMobileResolutionQualityLimits(
	TEXT("Lyra.DeviceProfile.Mobile.ResolutionQualityLimits"),
	TEXT(""),
	TEXT(
		"List of limits on resolution quality of the form \"FPS:MaxResQuality,FPS2:MaxResQuality2,...\", kicking in when FPS is at or above the threshold"),
	ECVF_Default | ECVF_Preview);

static TAutoConsoleVariable<FString> CVarMobileResolutionQualityRecommendation(
	TEXT("Lyra.DeviceProfile.Mobile.ResolutionQualityRecommendation"),
	TEXT("0:75"),
	TEXT(
		"List of limits on resolution quality of the form \"FPS:Recommendation,FPS2:Recommendation2,...\", kicking in when FPS is at or above the threshold"),
	ECVF_Default | ECVF_Preview);

//////////////////////////////////////////////////////////////////////

FLyraScalabilitySnapshot::FLyraScalabilitySnapshot()
{
	static_assert(sizeof(Scalability::FQualityLevels) == 88,
	              "This function may need to be updated to account for new members");

	Qualities.ResolutionQuality = -1.0f;
	Qualities.ViewDistanceQuality = -1;
	Qualities.AntiAliasingQuality = -1;
	Qualities.ShadowQuality = -1;
	Qualities.GlobalIlluminationQuality = -1;
	Qualities.ReflectionQuality = -1;
	Qualities.PostProcessQuality = -1;
	Qualities.TextureQuality = -1;
	Qualities.EffectsQuality = -1;
	Qualities.FoliageQuality = -1;
	Qualities.ShadingQuality = -1;
}

//////////////////////////////////////////////////////////////////////

template <typename T>
struct TMobileQualityWrapper
{
private:
	T DefaultValue;
	TAutoConsoleVariable<FString>& WatchedVar;
	FString LastSeenCVarString;

	struct FLimitPair
	{
		int32 Limit = 0;
		T Value = T(0);
	};

	TArray<FLimitPair> Thresholds;

public:
	TMobileQualityWrapper(T InDefaultValue, TAutoConsoleVariable<FString>& InWatchedVar)
		: DefaultValue(InDefaultValue)
		  , WatchedVar(InWatchedVar)
	{
	}

	T Query(int32 TestValue)
	{
		UpdateCache();

		for (const FLimitPair& Pair : Thresholds)
		{
			if (TestValue >= Pair.Limit)
			{
				return Pair.Value;
			}
		}

		return DefaultValue;
	}

	// Returns the first threshold value or INDEX_NONE if there aren't any
	int32 GetFirstThreshold()
	{
		UpdateCache();
		return (Thresholds.Num() > 0) ? Thresholds[0].Limit : INDEX_NONE;
	}

	// Returns the lowest value of all the pairs or DefaultIfNoPairs if there are no pairs
	T GetLowestValue(T DefaultIfNoPairs)
	{
		UpdateCache();

		T Result = DefaultIfNoPairs;
		bool bFirstValue = true;
		for (const FLimitPair& Pair : Thresholds)
		{
			if (bFirstValue)
			{
				Result = Pair.Value;
				bFirstValue = false;
			}
			else
			{
				Result = FMath::Min(Result, Pair.Value);
			}
		}

		return Result;
	}

private:
	void UpdateCache()
	{
		const FString CurrentValue = WatchedVar.GetValueOnGameThread();
		if (!CurrentValue.Equals(LastSeenCVarString, ESearchCase::CaseSensitive))
		{
			LastSeenCVarString = CurrentValue;

			Thresholds.Reset();

			// Parse the thresholds
			int32 ScanIndex = 0;
			while (ScanIndex < LastSeenCVarString.Len())
			{
				const int32 ColonIndex = LastSeenCVarString.Find(
					TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart, ScanIndex);
				if (ColonIndex > 0)
				{
					const int32 CommaIndex = LastSeenCVarString.Find(
						TEXT(","), ESearchCase::CaseSensitive, ESearchDir::FromStart, ColonIndex);
					const int32 EndOfPairIndex = (CommaIndex != INDEX_NONE) ? CommaIndex : LastSeenCVarString.Len();

					FLimitPair Pair;
					LexFromString(Pair.Limit, *LastSeenCVarString.Mid(ScanIndex, ColonIndex - ScanIndex));
					LexFromString(Pair.Value, *LastSeenCVarString.Mid(ColonIndex + 1, EndOfPairIndex - ColonIndex - 1));
					Thresholds.Add(Pair);

					ScanIndex = EndOfPairIndex + 1;
				}
				else
				{
					UE_LOG(LogConsoleResponse, Error, TEXT("Malformed value for '%s'='%s', expecting a ':'"),
					       *IConsoleManager::Get().FindConsoleObjectName(WatchedVar.AsVariable()),
					       *LastSeenCVarString);
					Thresholds.Reset();
					break;
				}
			}

			// Sort the pairs
			Thresholds.Sort([](const FLimitPair A, const FLimitPair B) { return A.Limit < B.Limit; });
		}
	}
};

namespace LyraSettingsHelpers
{
	bool HasPlatformTrait(const FGameplayTag Tag)
	{
		return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(Tag);
	}

	// Returns the max level from the integer scalability settings (ignores ResolutionQuality)
	int32 GetHighestLevelOfAnyScalabilityChannel(const Scalability::FQualityLevels& ScalabilityQuality)
	{
		static_assert(sizeof(Scalability::FQualityLevels) == 88,
		              "This function may need to be updated to account for new members");

		int32 MaxScalability = ScalabilityQuality.ViewDistanceQuality;
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.AntiAliasingQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ShadowQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.GlobalIlluminationQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ReflectionQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.PostProcessQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.TextureQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.EffectsQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.FoliageQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ShadingQuality);

		return (MaxScalability >= 0) ? MaxScalability : -1;
	}

	void FillScalabilitySettingsFromDeviceProfile(FLyraScalabilitySnapshot& Mode, const FString& Suffix = FString())
	{
		static_assert(sizeof(Scalability::FQualityLevels) == 88,
		              "This function may need to be updated to account for new members");

		// Default out before filling so we can correctly mark non-overridden scalability values.
		// It's technically possible to swap device profile when testing so safest to clear and refill
		Mode = FLyraScalabilitySnapshot();

		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.ResolutionQuality%s"), *Suffix), Mode.Qualities.ResolutionQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.ViewDistanceQuality%s"), *Suffix), Mode.Qualities.ViewDistanceQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.AntiAliasingQuality%s"), *Suffix), Mode.Qualities.AntiAliasingQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.ShadowQuality%s"), *Suffix), Mode.Qualities.ShadowQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.GlobalIlluminationQuality%s"), *Suffix), Mode.Qualities.GlobalIlluminationQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.ReflectionQuality%s"), *Suffix), Mode.Qualities.ReflectionQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.PostProcessQuality%s"), *Suffix), Mode.Qualities.PostProcessQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.TextureQuality%s"), *Suffix), Mode.Qualities.TextureQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.EffectsQuality%s"), *Suffix), Mode.Qualities.EffectsQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.FoliageQuality%s"), *Suffix), Mode.Qualities.FoliageQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(
			FString::Printf(TEXT("sg.ShadingQuality%s"), *Suffix), Mode.Qualities.ShadingQuality);
	}

	TMobileQualityWrapper<int32> OverallQualityLimits(-1, CVarMobileQualityLimits);
	TMobileQualityWrapper<float> ResolutionQualityLimits(100.0f, CVarMobileResolutionQualityLimits);
	TMobileQualityWrapper<float> ResolutionQualityRecommendations(75.0f, CVarMobileResolutionQualityRecommendation);

	int32 GetApplicableOverallQualityLimit(const int32 FrameRate)
	{
		return OverallQualityLimits.Query(FrameRate);
	}

	float GetApplicableResolutionQualityLimit(const int32 FrameRate)
	{
		return ResolutionQualityLimits.Query(FrameRate);
	}

	float GetApplicableResolutionQualityRecommendation(const int32 FrameRate)
	{
		return ResolutionQualityRecommendations.Query(FrameRate);
	}

	int32 ConstrainFrameRateToBeCompatibleWithOverallQuality(const int32 FrameRate, const int32 OverallQuality)
	{
		const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
		const TArray<int32>& PossibleRates = PlatformSettings->MobileFrameRateLimits;

		// Choose the closest frame rate (without going over) to the user preferred one that is supported and compatible with the desired overall quality
		int32 LimitIndex = PossibleRates.FindLastByPredicate([=](const int32& TestRate)
		{
			const bool bAtOrBelowDesiredRate = (TestRate <= FrameRate);

			const int32 LimitQuality = GetApplicableResolutionQualityLimit(TestRate);
			const bool bQualityDoesntExceedLimit = LimitQuality < 0 || OverallQuality <= LimitQuality;

			const bool bIsSupported = ULyraSettingsLocal::IsSupportedMobileFramePace(TestRate);

			return bAtOrBelowDesiredRate && bQualityDoesntExceedLimit && bIsSupported;
		});

		return PossibleRates.IsValidIndex(LimitIndex)
			       ? PossibleRates[LimitIndex]
			       : ULyraSettingsLocal::GetDefaultMobileFrameRate();
	}

	// Returns the first frame rate at which overall quality is restricted/limited by the current device profile
	int32 GetFirstFrameRateWithQualityLimit()
	{
		return OverallQualityLimits.GetFirstThreshold();
	}

	// Returns the lowest quality at which there's a limit on the overall frame rate (or -1 if there is no limit)
	int32 GetLowestQualityWithFrameRateLimit()
	{
		return OverallQualityLimits.GetLowestValue(-1);
	}
}

//////////////////////////////////////////////////////////////////////

PRAGMA_DISABLE_DEPRECATION_WARNINGS

ULyraSettingsLocal::ULyraSettingsLocal()
{
	if (!HasAnyFlags(RF_ClassDefaultObject) && FSlateApplication::IsInitialized())
	{
		OnApplicationActivationStateChangedHandle = FSlateApplication::Get()
		                                            .OnApplicationActivationStateChanged()
		                                            .AddUObject(this, &ThisClass::OnAppActivationStateChanged);
	}

	ULyraSettingsLocal::SetToDefaults();
}

// PRAGMA_ENABLE_DEPRECATION_WARNINGS

void ULyraSettingsLocal::SetToDefaults()
{
	Super::SetToDefaults();

	bUseHeadphoneMode = false;
	bUseHDRAudioMode = false;
	bSoundControlBusMixLoaded = false;

	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	UserChosenDeviceProfileSuffix = PlatformSettings->DefaultDeviceProfileSuffix;
	DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;

	FrameRateLimit_InMenu = 144.0f;
	FrameRateLimit_WhenBackgrounded = 30.0f;
	FrameRateLimit_OnBattery = 60.0f;

	MobileFrameRateLimit = GetDefaultMobileFrameRate();
	DesiredMobileFrameRateLimit = MobileFrameRateLimit;
}

void ULyraSettingsLocal::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);

	// Console platforms use rhi.SyncInterval to limit framerate
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode == ELyraFramePacingMode::ConsoleStyle)
		FrameRateLimit = 0.0f;

	// Enable HRTF if needed
	bDesiredHeadphoneMode = bUseHeadphoneMode;
	SetHeadphoneModeEnabled(bUseHeadphoneMode);

	DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;

	LyraSettingsHelpers::FillScalabilitySettingsFromDeviceProfile(DeviceDefaultScalabilitySettings);
	DesiredMobileFrameRateLimit = MobileFrameRateLimit;
	ClampMobileQuality();

	PerfStatSettingsChangedEvent.Broadcast();
}

void ULyraSettingsLocal::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();

	bDesiredHeadphoneMode = bUseHeadphoneMode;
	UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;
	MobileFrameRateLimit = DesiredMobileFrameRateLimit;
}

void ULyraSettingsLocal::BeginDestroy()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().
		                         Remove(OnApplicationActivationStateChangedHandle);
	}

	Super::BeginDestroy();
}

ULyraSettingsLocal* ULyraSettingsLocal::Get()
{
	return GEngine ? CastChecked<ULyraSettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}

void ULyraSettingsLocal::ConfirmVideoMode()
{
	Super::ConfirmVideoMode();
	SetMobileFPSMode(DesiredMobileFrameRateLimit);
}
 
// Combines two limits, always taking the minimum of the two (with special handling for values of <= 0 meaning unlimited)
float CombineFrameRateLimits(const float Limit1, const float Limit2)
{
	if (Limit1 <= 0.0f) return Limit2;
	if (Limit2 <= 0.0f) return Limit1;
	return FMath::Min(Limit1, Limit2);
}

float ULyraSettingsLocal::GetEffectiveFrameRateLimit()
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();

#if WITH_EDITOR
	if (GIsEditor && !CVarApplyFrameRateSettingsInPIE.GetValueOnGameThread())
		return Super::GetEffectiveFrameRateLimit();
#endif

	if (PlatformSettings->FramePacingMode == ELyraFramePacingMode::ConsoleStyle)
		return 0.0f;

	float EffectiveFrameRateLimit = Super::GetEffectiveFrameRateLimit();

	if (ShouldUseFrontendPerformanceSettings())
		EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_InMenu);

	if (PlatformSettings->FramePacingMode != ELyraFramePacingMode::DesktopStyle)
		return EffectiveFrameRateLimit;

	if (FPlatformMisc::IsRunningOnBattery())
		EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_OnBattery);

	if (FSlateApplication::IsInitialized() && !FSlateApplication::Get().IsActive())
		EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_WhenBackgrounded);

	return EffectiveFrameRateLimit;
}

int32 ULyraSettingsLocal::GetHighestLevelOfAnyScalabilityChannel() const
{
	return LyraSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel(ScalabilityQuality);
}

void ULyraSettingsLocal::OverrideQualityLevelsToScalabilityMode(const FLyraScalabilitySnapshot& InMode,
                                                                Scalability::FQualityLevels& InOutLevels) const
{
	static_assert(sizeof(Scalability::FQualityLevels) == 88,
	              "This function may need to be updated to account for new members");

	// Overrides any valid (non-negative) settings
	InOutLevels.ResolutionQuality = (InMode.Qualities.ResolutionQuality >= 0.f)
		                                ? InMode.Qualities.ResolutionQuality
		                                : InOutLevels.ResolutionQuality;
	InOutLevels.ViewDistanceQuality = (InMode.Qualities.ViewDistanceQuality >= 0)
		                                  ? InMode.Qualities.ViewDistanceQuality
		                                  : InOutLevels.ViewDistanceQuality;
	InOutLevels.AntiAliasingQuality = (InMode.Qualities.AntiAliasingQuality >= 0)
		                                  ? InMode.Qualities.AntiAliasingQuality
		                                  : InOutLevels.AntiAliasingQuality;
	InOutLevels.ShadowQuality = (InMode.Qualities.ShadowQuality >= 0)
		                            ? InMode.Qualities.ShadowQuality
		                            : InOutLevels.ShadowQuality;
	InOutLevels.GlobalIlluminationQuality = (InMode.Qualities.GlobalIlluminationQuality >= 0)
		                                        ? InMode.Qualities.GlobalIlluminationQuality
		                                        : InOutLevels.GlobalIlluminationQuality;
	InOutLevels.ReflectionQuality = (InMode.Qualities.ReflectionQuality >= 0)
		                                ? InMode.Qualities.ReflectionQuality
		                                : InOutLevels.ReflectionQuality;
	InOutLevels.PostProcessQuality = (InMode.Qualities.PostProcessQuality >= 0)
		                                 ? InMode.Qualities.PostProcessQuality
		                                 : InOutLevels.PostProcessQuality;
	InOutLevels.TextureQuality = (InMode.Qualities.TextureQuality >= 0)
		                             ? InMode.Qualities.TextureQuality
		                             : InOutLevels.TextureQuality;
	InOutLevels.EffectsQuality = (InMode.Qualities.EffectsQuality >= 0)
		                             ? InMode.Qualities.EffectsQuality
		                             : InOutLevels.EffectsQuality;
	InOutLevels.FoliageQuality = (InMode.Qualities.FoliageQuality >= 0)
		                             ? InMode.Qualities.FoliageQuality
		                             : InOutLevels.FoliageQuality;
	InOutLevels.ShadingQuality = (InMode.Qualities.ShadingQuality >= 0)
		                             ? InMode.Qualities.ShadingQuality
		                             : InOutLevels.ShadingQuality;
}

void ULyraSettingsLocal::ClampQualityLevelsToDeviceProfile(const Scalability::FQualityLevels& ClampLevels,
                                                           Scalability::FQualityLevels& InOutLevels) const
{
	static_assert(sizeof(Scalability::FQualityLevels) == 88,
	              "This function may need to be updated to account for new members");

	// Clamps any valid (non-negative) settings
	InOutLevels.ResolutionQuality = (ClampLevels.ResolutionQuality >= 0.f)
		                                ? FMath::Min(ClampLevels.ResolutionQuality, InOutLevels.ResolutionQuality)
		                                : InOutLevels.ResolutionQuality;
	InOutLevels.ViewDistanceQuality = (ClampLevels.ViewDistanceQuality >= 0)
		                                  ? FMath::Min(ClampLevels.ViewDistanceQuality, InOutLevels.ViewDistanceQuality)
		                                  : InOutLevels.ViewDistanceQuality;
	InOutLevels.AntiAliasingQuality = (ClampLevels.AntiAliasingQuality >= 0)
		                                  ? FMath::Min(ClampLevels.AntiAliasingQuality, InOutLevels.AntiAliasingQuality)
		                                  : InOutLevels.AntiAliasingQuality;
	InOutLevels.ShadowQuality = (ClampLevels.ShadowQuality >= 0)
		                            ? FMath::Min(ClampLevels.ShadowQuality, InOutLevels.ShadowQuality)
		                            : InOutLevels.ShadowQuality;
	InOutLevels.GlobalIlluminationQuality = (ClampLevels.GlobalIlluminationQuality >= 0)
		                                        ? FMath::Min(ClampLevels.GlobalIlluminationQuality,
		                                                     InOutLevels.GlobalIlluminationQuality)
		                                        : InOutLevels.GlobalIlluminationQuality;
	InOutLevels.ReflectionQuality = (ClampLevels.ReflectionQuality >= 0)
		                                ? FMath::Min(ClampLevels.ReflectionQuality, InOutLevels.ReflectionQuality)
		                                : InOutLevels.ReflectionQuality;
	InOutLevels.PostProcessQuality = (ClampLevels.PostProcessQuality >= 0)
		                                 ? FMath::Min(ClampLevels.PostProcessQuality, InOutLevels.PostProcessQuality)
		                                 : InOutLevels.PostProcessQuality;
	InOutLevels.TextureQuality = (ClampLevels.TextureQuality >= 0)
		                             ? FMath::Min(ClampLevels.TextureQuality, InOutLevels.TextureQuality)
		                             : InOutLevels.TextureQuality;
	InOutLevels.EffectsQuality = (ClampLevels.EffectsQuality >= 0)
		                             ? FMath::Min(ClampLevels.EffectsQuality, InOutLevels.EffectsQuality)
		                             : InOutLevels.EffectsQuality;
	InOutLevels.FoliageQuality = (ClampLevels.FoliageQuality >= 0)
		                             ? FMath::Min(ClampLevels.FoliageQuality, InOutLevels.FoliageQuality)
		                             : InOutLevels.FoliageQuality;
	InOutLevels.ShadingQuality = (ClampLevels.ShadingQuality >= 0)
		                             ? FMath::Min(ClampLevels.ShadingQuality, InOutLevels.ShadingQuality)
		                             : InOutLevels.ShadingQuality;
}

void ULyraSettingsLocal::OnExperienceLoaded()
{
	ReapplyThingsDueToPossibleDeviceProfileChange();
}

void ULyraSettingsLocal::OnHotfixDeviceProfileApplied()
{
	ReapplyThingsDueToPossibleDeviceProfileChange();
}

void ULyraSettingsLocal::ReapplyThingsDueToPossibleDeviceProfileChange()
{
	ApplyNonResolutionSettings();
}

void ULyraSettingsLocal::SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd)
{
	bInFrontEndForPerformancePurposes = bInFrontEnd;
	UpdateEffectiveFrameRateLimit();
}

bool ULyraSettingsLocal::ShouldUseFrontendPerformanceSettings() const
{
#if WITH_EDITOR
	if (GIsEditor && !CVarApplyFrontEndPerformanceOptionsInPIE.GetValueOnGameThread())
	{
		return false;
	}
#endif

	return bInFrontEndForPerformancePurposes;
}

ELyraStatDisplayMode ULyraSettingsLocal::GetPerfStatDisplayState(ELyraDisplayablePerformanceStat Stat) const
{
	if (const ELyraStatDisplayMode* DisplayMode = DisplayStatList.Find(Stat))
	{
		return *DisplayMode;
	}

	return ELyraStatDisplayMode::Hidden;
}

void ULyraSettingsLocal::SetPerfStatDisplayState(ELyraDisplayablePerformanceStat Stat, ELyraStatDisplayMode DisplayMode)
{
	if (DisplayMode == ELyraStatDisplayMode::Hidden)
	{
		DisplayStatList.Remove(Stat);
	}
	else
	{
		DisplayStatList.FindOrAdd(Stat) = DisplayMode;
	}

	PerfStatSettingsChangedEvent.Broadcast();
}


void ULyraSettingsLocal::SetDisplayGamma(const float InGamma)
{
	DisplayGamma = InGamma;
	ApplyDisplayGamma();
}

void ULyraSettingsLocal::ApplyDisplayGamma() const
{
	if (GEngine)
	{
		GEngine->DisplayGamma = DisplayGamma;
	}
}


void ULyraSettingsLocal::SetFrameRateLimit_OnBattery(const float NewLimitFPS)
{
	FrameRateLimit_OnBattery = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}


void ULyraSettingsLocal::SetFrameRateLimit_InMenu(float NewLimitFPS)
{
	FrameRateLimit_InMenu = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}


void ULyraSettingsLocal::SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS)
{
	FrameRateLimit_WhenBackgrounded = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}

float ULyraSettingsLocal::GetFrameRateLimit_Always() const
{
	return GetFrameRateLimit();
}

void ULyraSettingsLocal::SetFrameRateLimit_Always(float NewLimitFPS)
{
	SetFrameRateLimit(NewLimitFPS);
	UpdateEffectiveFrameRateLimit();
}

void ULyraSettingsLocal::UpdateEffectiveFrameRateLimit()
{
	if (!IsRunningDedicatedServer())
	{
		SetFrameRateLimitCVar(GetEffectiveFrameRateLimit());
	}
}

int32 ULyraSettingsLocal::GetDefaultMobileFrameRate()
{
	return CVarDeviceProfileDrivenMobileDefaultFrameRate.GetValueOnGameThread();
}

int32 ULyraSettingsLocal::GetMaxMobileFrameRate()
{
	return CVarDeviceProfileDrivenMobileMaxFrameRate.GetValueOnGameThread();
}

bool ULyraSettingsLocal::IsSupportedMobileFramePace(const int32 TestFPS)
{
	const bool bIsDefault = (TestFPS == GetDefaultMobileFrameRate());
	const bool bDoesNotExceedLimit = (TestFPS <= GetMaxMobileFrameRate());

	// Allow all paces in the editor, as we'd only be doing this when simulating another platform
	const bool bIsSupportedPace = FPlatformRHIFramePacer::SupportsFramePace(TestFPS) || GIsEditor;

	return bIsDefault || (bDoesNotExceedLimit && bIsSupportedPace);
}

int32 ULyraSettingsLocal::GetFirstFrameRateWithQualityLimit() const
{
	return LyraSettingsHelpers::GetFirstFrameRateWithQualityLimit();
}

int32 ULyraSettingsLocal::GetLowestQualityWithFrameRateLimit() const
{
	return LyraSettingsHelpers::GetLowestQualityWithFrameRateLimit();
}

void ULyraSettingsLocal::ResetToMobileDeviceDefaults()
{
	// Reset frame rate
	DesiredMobileFrameRateLimit = GetDefaultMobileFrameRate();
	MobileFrameRateLimit = DesiredMobileFrameRateLimit;

	// Reset scalability
	Scalability::FQualityLevels DefaultLevels = Scalability::GetQualityLevels();
	OverrideQualityLevelsToScalabilityMode(DeviceDefaultScalabilitySettings, DefaultLevels);
	ScalabilityQuality = DefaultLevels;

	// Apply
	UpdateGameModeDeviceProfileAndFps();
}

int32 ULyraSettingsLocal::GetMaxSupportedOverallQualityLevel() const
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	return PlatformSettings->FramePacingMode == ELyraFramePacingMode::MobileStyle
	       && DeviceDefaultScalabilitySettings.bHasOverrides
		       ? LyraSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel(DeviceDefaultScalabilitySettings.Qualities)
		       : 3;
}

void ULyraSettingsLocal::SetMobileFPSMode(const int32 NewLimitFPS)
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode != ELyraFramePacingMode::MobileStyle) return;

	if (MobileFrameRateLimit != NewLimitFPS)
	{
		MobileFrameRateLimit = NewLimitFPS;
		UpdateGameModeDeviceProfileAndFps();
	}

	DesiredMobileFrameRateLimit = MobileFrameRateLimit;
}

void ULyraSettingsLocal::SetDesiredMobileFrameRateLimit(const int32 NewLimitFPS)
{
	const int32 OldLimitFPS = DesiredMobileFrameRateLimit;
	RemapMobileResolutionQuality(OldLimitFPS, NewLimitFPS);

	DesiredMobileFrameRateLimit = NewLimitFPS;

	ClampMobileFPSQualityLevels(/*bWriteBack=*/ false);
}

void ULyraSettingsLocal::ClampMobileFPSQualityLevels(const bool bWriteBack)
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode != ELyraFramePacingMode::MobileStyle) return;
	const int32 QualityLimit = LyraSettingsHelpers::GetApplicableOverallQualityLimit(DesiredMobileFrameRateLimit);
	const int32 CurrentQualityLevel = GetHighestLevelOfAnyScalabilityChannel();

	if (QualityLimit < 0 || CurrentQualityLevel <= QualityLimit)
		return;
	SetOverallScalabilityLevel(QualityLimit);

	if (bWriteBack)
		SetQualityLevels(ScalabilityQuality);

	UE_LOG(LogConsoleResponse, Log, TEXT("Mobile FPS clamped overall quality (%d -> %d)."), CurrentQualityLevel,
	       QualityLimit);
}

void ULyraSettingsLocal::ClampMobileQuality()
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode != ELyraFramePacingMode::MobileStyle) return;

	// Clamp the resultant settings to the device default, it's known viable maximum.
	// This is a clamp rather than override to preserve allowed user settings
	Scalability::FQualityLevels CurrentLevels = Scalability::GetQualityLevels();

	/** On mobile, disables the 3D Resolution clamp that reverts the setting set by the user on boot.*/
	if (bMobileDisableResolutionReset)
		DeviceDefaultScalabilitySettings.Qualities.ResolutionQuality = CurrentLevels.ResolutionQuality;

	ClampQualityLevelsToDeviceProfile(DeviceDefaultScalabilitySettings.Qualities, /*inout*/ CurrentLevels);
	SetQualityLevels(CurrentLevels);

	// Clamp quality levels if required at the current frame rate
	ClampMobileFPSQualityLevels(/*bWriteBack=*/ true);

	const int32 MaxMobileFrameRate = GetMaxMobileFrameRate();
	const int32 DefaultMobileFrameRate = GetDefaultMobileFrameRate();

	ensureMsgf(DefaultMobileFrameRate <= MaxMobileFrameRate,
	           TEXT("Default mobile frame rate (%d) is higher than the maximum mobile frame rate (%d)!"),
	           DefaultMobileFrameRate, MaxMobileFrameRate);

	// Choose the closest supported frame rate to the user desired setting without going over the device imposed limit
	const TArray<int32>& PossibleRates = PlatformSettings->MobileFrameRateLimits;
	const int32 LimitIndex = PossibleRates.FindLastByPredicate([this](const int32& TestRate)
	{
		return (TestRate <= DesiredMobileFrameRateLimit) && IsSupportedMobileFramePace(TestRate);
	});
	const int32 ActualLimitFPS = PossibleRates.IsValidIndex(LimitIndex)
		                             ? PossibleRates[LimitIndex]
		                             : GetDefaultMobileFrameRate();

	ClampMobileResolutionQuality(ActualLimitFPS);
}

void ULyraSettingsLocal::ClampMobileResolutionQuality(const int32 TargetFPS)
{
	// Clamp mobile resolution quality
	const float MaxMobileResQuality = LyraSettingsHelpers::GetApplicableResolutionQualityLimit(TargetFPS);
	float CurrentScaleNormalized = 0.0f;
	float CurrentScaleValue = 0.0f;
	float MinScaleValue = 0.0f;
	float MaxScaleValue = 0.0f;

	GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScaleValue, MinScaleValue, MaxScaleValue);
	if (CurrentScaleValue > MaxMobileResQuality)
	{
		UE_LOG(LogConsoleResponse, Log, TEXT("clamping mobile resolution quality max res: %f, %f, %f, %f, %f"),
		       CurrentScaleNormalized, CurrentScaleValue, MinScaleValue, MaxScaleValue, MaxMobileResQuality);
		SetResolutionScaleValueEx(MaxMobileResQuality);
	}
}

void ULyraSettingsLocal::RemapMobileResolutionQuality(const int32 FromFPS, const int32 ToFPS)
{
	// Mobile resolution quality slider is a normalized value that is lerped between min quality, max quality.
	// max quality can change depending on FPS mode. This code remaps the quality when FPS mode changes so that the normalized
	// value remains the same within the new range.
	float CurrentScaleNormalized = 0.0f;
	float CurrentScaleValue = 0.0f;
	float MinScaleValue = 0.0f;
	float MaxScaleValue = 0.0f;

	GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScaleValue, MinScaleValue, MaxScaleValue);
	const float FromMaxMobileResQuality = LyraSettingsHelpers::GetApplicableResolutionQualityLimit(FromFPS);
	const float ToMaxMobileResQuality = LyraSettingsHelpers::GetApplicableResolutionQualityLimit(ToFPS);
	const float FromMobileScaledNormalizedValue = (CurrentScaleValue - MinScaleValue) / (FromMaxMobileResQuality -
		MinScaleValue);
	const float ToResQuality = FMath::Lerp(MinScaleValue, ToMaxMobileResQuality, FromMobileScaledNormalizedValue);

	UE_LOG(LogConsoleResponse, Log, TEXT("Remap mobile resolution quality %f, %f, (%d,%d)"),
	       CurrentScaleValue, ToResQuality, FromFPS, ToFPS);

	SetResolutionScaleValueEx(ToResQuality);
}


void ULyraSettingsLocal::SetHeadphoneModeEnabled(const bool bEnabled)
{
	if (!CanModifyHeadphoneModeEnabled()) return;

	static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(
		TEXT("au.DisableBinauralSpatialization"));

	if (!BinauralSpatializationDisabledCVar) return;

	BinauralSpatializationDisabledCVar->Set(!bEnabled, ECVF_SetByGameSetting);

	// Only save settings if the setting actually changed
	if (bUseHeadphoneMode == bEnabled) return;

	bUseHeadphoneMode = bEnabled;
	SaveSettings();
}

bool ULyraSettingsLocal::CanModifyHeadphoneModeEnabled() const
{
	static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(
		TEXT("au.DisableBinauralSpatialization"));

	const bool bHRTFOptionAvailable = BinauralSpatializationDisabledCVar &&
		(BinauralSpatializationDisabledCVar->GetFlags() & ECVF_SetByMask) <=
		ECVF_SetByGameSetting;

	const bool bBinauralSettingControlledByOS = LyraSettingsHelpers::HasPlatformTrait(
		TAG_Platform_Trait_BinauralSettingControlledByOS);

	return bHRTFOptionAvailable && !bBinauralSettingControlledByOS;
}

void ULyraSettingsLocal::SetHDRAudioModeEnabled(const bool bEnabled)
{
	bUseHDRAudioMode = bEnabled;

	if (!GEngine) return;
	const UWorld* World = GEngine->GetCurrentPlayWorld();
	if (!World) return;

	if (const auto LyraAudioMixEffectsSubsystem = World->GetSubsystem<
		ULyraAudioMixEffectsSubsystem>())
		LyraAudioMixEffectsSubsystem->ApplyDynamicRangeEffectsChains(bEnabled);
}

bool ULyraSettingsLocal::CanRunAutoBenchmark() const
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	return PlatformSettings && PlatformSettings->bSupportsAutomaticVideoQualityBenchmark;
}

bool ULyraSettingsLocal::ShouldRunAutoBenchmarkAtStartup() const
{
	// can run Benchmark Or didn't run or loaded
	return CanRunAutoBenchmark() && LastCPUBenchmarkResult == -1;
}

void ULyraSettingsLocal::RunAutoBenchmark(const bool bSaveImmediately)
{
	RunHardwareBenchmark();

	// Always apply, optionally save
	ApplyScalabilitySettings();

	if (bSaveImmediately)
		SaveSettings();
}

void ULyraSettingsLocal::ApplyScalabilitySettings() const
{
	SetQualityLevels(ScalabilityQuality);
}

void ULyraSettingsLocal::SetOverallVolume(const float InVolume)
{
	OverallVolume = InVolume;
	SetVolume("Overall", OverallVolume);
}

void ULyraSettingsLocal::SetMusicVolume(const float InVolume)
{
	MusicVolume = InVolume;
	SetVolume("Music", MusicVolume);
}

void ULyraSettingsLocal::SetSoundFXVolume(const float InVolume)
{
	SoundFXVolume = InVolume;
	SetVolume("SoundFX", SoundFXVolume);
}

void ULyraSettingsLocal::SetDialogueVolume(const float InVolume)
{
	DialogueVolume = InVolume;
	SetVolume("Dialogue", DialogueVolume);
}

void ULyraSettingsLocal::SetVoiceChatVolume(const float InVolume)
{
	VoiceChatVolume = InVolume;
	SetVolume("VoiceChat", VoiceChatVolume);
}

void ULyraSettingsLocal::SetVolumeForControlBus(USoundControlBus* InSoundControlBus, const float InVolume)
{
	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called
	if (!bSoundControlBusMixLoaded)
		LoadUserControlBusMix();

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Assuming everything has been loaded correctly, we retrieve the world and use AudioModulationStatics to update the Control Bus Volume values and
	// apply the settings to the cached User Control Bus Mix
	if (!GEngine || !InSoundControlBus || !bSoundControlBusMixLoaded)
		return;

	if (const UWorld* AudioWorld = GEngine->GetCurrentPlayWorld())
	{
		ensureMsgf(ControlBusMix, TEXT("Control Bus Mix failed to load."));

		// Create and set the Control Bus Mix Stage Parameters
		FSoundControlBusMixStage UpdatedControlBusMixStage;
		UpdatedControlBusMixStage.Bus = InSoundControlBus;
		UpdatedControlBusMixStage.Value.TargetValue = InVolume;
		UpdatedControlBusMixStage.Value.AttackTime = 0.01f;
		UpdatedControlBusMixStage.Value.ReleaseTime = 0.01f;

		// Add the Control Bus Mix Stage to an Array as the UpdateMix function requires
		TArray<FSoundControlBusMixStage> UpdatedMixStageArray;
		UpdatedMixStageArray.Add(UpdatedControlBusMixStage);

		// Modify the matching bus Mix Stage parameters on the User Control Bus Mix
		UAudioModulationStatics::UpdateMix(AudioWorld, ControlBusMix, UpdatedMixStageArray);
	}
}

void ULyraSettingsLocal::SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId)
{
	AudioOutputDeviceId = InAudioOutputDeviceId;
	OnAudioOutputDeviceChanged.Broadcast(InAudioOutputDeviceId);
}

void ULyraSettingsLocal::ApplySafeZoneScale() const
{
	SSafeZone::SetGlobalSafeZoneScale(GetSafeZone());
}

void ULyraSettingsLocal::SetVolume(const FString& VolumeType, const float InVolume)
{
	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
		LoadUserControlBusMix();

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(*VolumeType);
	if (!ControlBusDblPtr) return;

	if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		SetVolumeForControlBus(ControlBusPtr, InVolume);
}

void ULyraSettingsLocal::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	// In this section, update each Control Bus to the currently cached UI settings
	SetVolume("Overall", OverallVolume);
	SetVolume("Music", MusicVolume);
	SetVolume("SoundFX", SoundFXVolume);
	SetVolume("Dialogue", DialogueVolume);
	SetVolume("VoiceChat", VoiceChatVolume);


	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
		InputSubsystem->SetGamepadInputType(ControllerPlatform);

	if (bUseHeadphoneMode != bDesiredHeadphoneMode)
		SetHeadphoneModeEnabled(bDesiredHeadphoneMode);

	if (DesiredUserChosenDeviceProfileSuffix != UserChosenDeviceProfileSuffix)
		UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;

	if (FApp::CanEverRender())
	{
		ApplyDisplayGamma();
		ApplySafeZoneScale();
		UpdateGameModeDeviceProfileAndFps();
	}

	PerfStatSettingsChangedEvent.Broadcast();
}

int32 ULyraSettingsLocal::GetOverallScalabilityLevel() const
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();

	return PlatformSettings->FramePacingMode == ELyraFramePacingMode::MobileStyle
		       ? GetHighestLevelOfAnyScalabilityChannel()
		       : Super::GetOverallScalabilityLevel();
}

void ULyraSettingsLocal::SetOverallScalabilityLevel(int32 Value)
{
	TGuardValue Guard(bSettingOverallQualityGuard, true);
	constexpr int MaxScalabilityLevel = 3;
	Value = FMath::Clamp(Value, 0, MaxScalabilityLevel);

	const float CurrentMobileResolutionQuality = ScalabilityQuality.ResolutionQuality;

	Super::SetOverallScalabilityLevel(Value);

	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode != ELyraFramePacingMode::MobileStyle)
		return;

	// Restore the resolution quality, mobile decouples this from overall quality
	ScalabilityQuality.ResolutionQuality = CurrentMobileResolutionQuality;

	// Changing the overall quality can end up adjusting the frame rate on mobile since there are limits
	const int32 ConstrainedFrameRateLimit = LyraSettingsHelpers::ConstrainFrameRateToBeCompatibleWithOverallQuality(
		DesiredMobileFrameRateLimit, Value);
	if (ConstrainedFrameRateLimit != DesiredMobileFrameRateLimit)
		SetDesiredMobileFrameRateLimit(ConstrainedFrameRateLimit);
}

void ULyraSettingsLocal::SetControllerPlatform(const FName InControllerPlatform)
{
	if (ControllerPlatform == InControllerPlatform)
		return;

	ControllerPlatform = InControllerPlatform;

	// Apply the change to the common input subsystem so that we refresh any input icons we're using.
	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
		InputSubsystem->SetGamepadInputType(ControllerPlatform);
}

//
// PRAGMA_DISABLE_DEPRECATION_WARNINGS
//
// void ULyraSettingsLocal::RegisterInputConfig(ECommonInputType Type, const UPlayerMappableInputConfig* NewConfig,
//                                              const bool bIsActive)
// {
// 	if (NewConfig)
// 	{
// 		const int32 ExistingConfigIdx = RegisteredInputConfigs.IndexOfByPredicate(
// 			[&NewConfig](const FLoadedMappableConfigPair& Pair) { return Pair.Config == NewConfig; });
// 		if (ExistingConfigIdx == INDEX_NONE)
// 		{
// 			const int32 NumAdded = RegisteredInputConfigs.Add(FLoadedMappableConfigPair(NewConfig, Type, bIsActive));
// 			if (NumAdded != INDEX_NONE)
// 			{
// 				OnInputConfigRegistered.Broadcast(RegisteredInputConfigs[NumAdded]);
// 			}
// 		}
// 	}
// }
//
// int32 ULyraSettingsLocal::UnregisterInputConfig(const UPlayerMappableInputConfig* ConfigToRemove)
// {
// 	if (ConfigToRemove)
// 	{
// 		const int32 Index = RegisteredInputConfigs.IndexOfByPredicate(
// 			[&ConfigToRemove](const FLoadedMappableConfigPair& Pair) { return Pair.Config == ConfigToRemove; });
// 		if (Index != INDEX_NONE)
// 		{
// 			RegisteredInputConfigs.RemoveAt(Index);
// 			return 1;
// 		}
// 	}
// 	return INDEX_NONE;
// }
//
// const UPlayerMappableInputConfig* ULyraSettingsLocal::GetInputConfigByName(FName ConfigName) const
// {
// 	for (const FLoadedMappableConfigPair& Pair : RegisteredInputConfigs)
// 	{
// 		if (Pair.Config->GetConfigName() == ConfigName)
// 		{
// 			return Pair.Config;
// 		}
// 	}
// 	return nullptr;
// }
//
// void ULyraSettingsLocal::GetRegisteredInputConfigsOfType(ECommonInputType Type,
//                                                          TArray<FLoadedMappableConfigPair>& OutArray) const
// {
// 	OutArray.Empty();
//
// 	// If "Count" is passed in then
// 	if (Type == ECommonInputType::Count)
// 	{
// 		OutArray = RegisteredInputConfigs;
// 		return;
// 	}
//
// 	for (const FLoadedMappableConfigPair& Pair : RegisteredInputConfigs)
// 	{
// 		if (Pair.Type == Type)
// 		{
// 			OutArray.Emplace(Pair);
// 		}
// 	}
// }
//
// void ULyraSettingsLocal::GetAllMappingNamesFromKey(const FKey InKey, TArray<FName>& OutActionNames)
// {
// 	if (InKey == EKeys::Invalid)
// 	{
// 		return;
// 	}
//
// 	// adding any names of actions that are bound to that key
// 	for (const FLoadedMappableConfigPair& Pair : RegisteredInputConfigs)
// 	{
// 		if (Pair.Type == ECommonInputType::MouseAndKeyboard)
// 		{
// 			for (const FEnhancedActionKeyMapping& Mapping : Pair.Config->GetPlayerMappableKeys())
// 			{
// 				FName MappingName(Mapping.GetDisplayName().ToString());
// 				FName ActionName = Mapping.GetMappingName();
// 				// make sure it isn't custom bound as well
// 				if (const FKey* MappingKey = CustomKeyboardConfig.Find(ActionName))
// 				{
// 					if (*MappingKey == InKey)
// 					{
// 						OutActionNames.Add(MappingName);
// 					}
// 				}
// 				else
// 				{
// 					if (Mapping.Key == InKey)
// 					{
// 						OutActionNames.Add(MappingName);
// 					}
// 				}
// 			}
// 		}
// 	}
// }
//
// void ULyraSettingsLocal::AddOrUpdateCustomKeyboardBindings(const FName MappingName, const FKey NewKey,
//                                                            ULocalPlayer* LocalPlayer)
// {
// 	if (MappingName == NAME_None)
// 	{
// 		return;
// 	}
//
// 	if (InputConfigName != TEXT("Custom"))
// 	{
// 		// Copy Presets.
// 		if (const UPlayerMappableInputConfig* DefaultConfig = GetInputConfigByName(TEXT("Default")))
// 		{
// 			for (const FEnhancedActionKeyMapping& Mapping : DefaultConfig->GetPlayerMappableKeys())
// 			{
// 				// Make sure that the mapping has a valid name, it's possible to have an empty name
// 				// if someone has marked a mapping as "Player Mappable" but deleted the default field value
// 				if (Mapping.GetMappingName() != NAME_None)
// 				{
// 					CustomKeyboardConfig.Add(Mapping.GetMappingName(), Mapping.Key);
// 				}
// 			}
// 		}
//
// 		InputConfigName = TEXT("Custom");
// 	}
//
// 	if (FKey* ExistingMapping = CustomKeyboardConfig.Find(MappingName))
// 	{
// 		// Change the key to the new one
// 		CustomKeyboardConfig[MappingName] = NewKey;
// 	}
// 	else
// 	{
// 		CustomKeyboardConfig.Add(MappingName, NewKey);
// 	}
//
// 	// Tell the enhanced input subsystem for this local player that we should remap some input! Woo
// 	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
// 		UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
// 	{
// 		Subsystem->AddPlayerMappedKeyInSlot(MappingName, NewKey);
// 	}
// }
//
// void ULyraSettingsLocal::ResetKeybindingToDefault(const FName MappingName, ULocalPlayer* LocalPlayer)
// {
// 	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
// 		UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
// 	{
// 		Subsystem->RemoveAllPlayerMappedKeysForMapping(MappingName);
// 	}
// }
//
// void ULyraSettingsLocal::ResetKeybindingsToDefault(ULocalPlayer* LocalPlayer)
// {
// 	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
// 		UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
// 	{
// 		Subsystem->RemoveAllPlayerMappedKeys();
// 	}
// }
//
// PRAGMA_ENABLE_DEPRECATION_WARNINGS
USoundControlBus* ULyraSettingsLocal::LoadControlBus(UObject* ObjPath, const FString& VolumeType)
{
	if (!ObjPath)return nullptr;
	const auto ControlBus = Cast<USoundControlBus>(ObjPath);

	if (!ControlBus)
	{
		ensureMsgf(ControlBus,
		           TEXT("%s Control Bus reference missing from Lyra Audio Settings."), *VolumeType);
		return nullptr;
	}

	ControlBusMap.Add(*VolumeType, ControlBus);
	return ControlBus;
}

void ULyraSettingsLocal::LoadUserControlBusMix()
{
	if (!GEngine) return;
	const UWorld* World = GEngine->GetCurrentPlayWorld();
	if (!World) return;
	const ULyraAudioSettings* AudioSettings = GetDefault<ULyraAudioSettings>();
	if (!AudioSettings) return;

	ControlBusMap.Empty();

	auto ObjPath = AudioSettings->OverallVolumeControlBus.TryLoad();
	auto OverallControlBus = LoadControlBus(ObjPath, "Overall");

	ObjPath = AudioSettings->OverallVolumeControlBus.TryLoad();
	auto MusicControlBus = LoadControlBus(ObjPath, "Music");

	ObjPath = AudioSettings->OverallVolumeControlBus.TryLoad();
	auto SoundFXControlBus = LoadControlBus(ObjPath, "SoundFX");

	ObjPath = AudioSettings->OverallVolumeControlBus.TryLoad();
	auto DialogueControlBus = LoadControlBus(ObjPath, "Dialogue");

	ObjPath = AudioSettings->OverallVolumeControlBus.TryLoad();
	auto VoiceChatControlBus = LoadControlBus(ObjPath, "VoiceChat");


	ObjPath = AudioSettings->UserSettingsControlBusMix.TryLoad();
	if (!ObjPath) return;
	USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath);
	if (!SoundControlBusMix)
	{
		ensureMsgf(SoundControlBusMix,
		           TEXT("User Settings Control Bus Mix reference missing from Lyra Audio Settings."));
		return;
	}

	ControlBusMix = SoundControlBusMix;

	const auto Overall_CBMStage =
		UAudioModulationStatics::CreateBusMixStage(World, OverallControlBus, OverallVolume);
	const auto Music_CBMStage =
		UAudioModulationStatics::CreateBusMixStage(World, MusicControlBus, MusicVolume);
	const auto SoundFX_CBMStage =
		UAudioModulationStatics::CreateBusMixStage(World, SoundFXControlBus, SoundFXVolume);
	const auto Dialogue_CBMStage =
		UAudioModulationStatics::CreateBusMixStage(World, DialogueControlBus, DialogueVolume);
	const auto VoiceChat_CBMStage =
		UAudioModulationStatics::CreateBusMixStage(World, VoiceChatControlBus, VoiceChatVolume);

	TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
	ControlBusMixStageArray.Add(Overall_CBMStage);
	ControlBusMixStageArray.Add(Music_CBMStage);
	ControlBusMixStageArray.Add(SoundFX_CBMStage);
	ControlBusMixStageArray.Add(Dialogue_CBMStage);
	ControlBusMixStageArray.Add(VoiceChat_CBMStage);

	UAudioModulationStatics::UpdateMix(World, ControlBusMix, ControlBusMixStageArray);

	bSoundControlBusMixLoaded = true;
}

void ULyraSettingsLocal::OnAppActivationStateChanged(bool bIsActive)
{
	// We might want to adjust the frame rate when the app loses/gains focus on multi-window platforms
	UpdateEffectiveFrameRateLimit();
}

void ULyraSettingsLocal::UpdateGameModeDeviceProfileAndFps()
{
#if WITH_EDITOR
	if (GIsEditor && !CVarApplyDeviceProfilesInPIE.GetValueOnGameThread())
	{
		return;
	}
#endif

	UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	const TArray<FLyraQualityDeviceProfileVariant>& UserFacingVariants = PlatformSettings->
		UserFacingDeviceProfileOptions;

	FString ExperienceSuffix;
	const int32 PlatformMaxRefreshRate = FPlatformMisc::GetMaxRefreshRate();
	const FString EffectiveUserSuffix = GetEffectiveUserSuffix(UserFacingVariants, PlatformMaxRefreshRate);

	FString BasePlatformName = GetBasePlatformName();
	FName PlatformName = GetPlatformName();

	TArray<FString> ComposedNamesToFind = BuildComposedNamesToFind(BasePlatformName, EffectiveUserSuffix,
	                                                               ExperienceSuffix);

	const FString ActualProfileToApply = FindActualProfileToApply(Manager, ComposedNamesToFind, PlatformName);

	UE_LOG(LogConsoleResponse, Log,
	       TEXT(
		       "UpdateGameModeDeviceProfileAndFps MaxRefreshRate=%d, ExperienceSuffix='%s', UserPicked='%s'->'%s', PlatformBase='%s', AppliedActual='%s'"
	       ),
	       PlatformMaxRefreshRate, *ExperienceSuffix, *UserChosenDeviceProfileSuffix, *EffectiveUserSuffix,
	       *BasePlatformName, *ActualProfileToApply);

	ApplyDeviceProfileIfNeeded(Manager, ActualProfileToApply);

	UpdateFramePacing(PlatformSettings->FramePacingMode);
}

FString ULyraSettingsLocal::GetEffectiveUserSuffix(const TArray<FLyraQualityDeviceProfileVariant>& UserFacingVariants,
                                                   const int32 PlatformMaxRefreshRate) const
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();

	int32 SuffixIndex = UserFacingVariants.IndexOfByPredicate([&](const FLyraQualityDeviceProfileVariant& Data)
	{
		return Data.DeviceProfileSuffix == UserChosenDeviceProfileSuffix;
	});
	while (UserFacingVariants.IsValidIndex(SuffixIndex))
	{
		if (PlatformMaxRefreshRate >= UserFacingVariants[SuffixIndex].MinRefreshRate)
			break;

		--SuffixIndex;
	}
	return UserFacingVariants.IsValidIndex(SuffixIndex)
		       ? UserFacingVariants[SuffixIndex].DeviceProfileSuffix
		       : PlatformSettings->DefaultDeviceProfileSuffix;
}

FString ULyraSettingsLocal::GetBasePlatformName() const
{
	FString BasePlatformName = UDeviceProfileManager::GetPlatformDeviceProfileName();
#if WITH_EDITOR
	if (GIsEditor)
	{
		const auto Settings = GetDefault<UPlatformEmulationSettings>();
		const FName PretendBaseDeviceProfile = Settings->GetPretendBaseDeviceProfile();
		if (PretendBaseDeviceProfile != NAME_None)
		{
			BasePlatformName = PretendBaseDeviceProfile.ToString();
		}
	}
#endif
	return BasePlatformName;
}

FName ULyraSettingsLocal::GetPlatformName() const
{
	FName PlatformName;
#if WITH_EDITOR
	if (GIsEditor)
	{
		const auto Settings = GetDefault<UPlatformEmulationSettings>();
		PlatformName = Settings->GetPretendPlatformName();
	}
#endif
	return PlatformName;
}

TArray<FString> ULyraSettingsLocal::BuildComposedNamesToFind(const FString& BasePlatformName,
                                                             const FString& EffectiveUserSuffix,
                                                             const FString& ExperienceSuffix) const
{
	TArray<FString> ComposedNamesToFind;
	const bool bHadUserSuffix = !EffectiveUserSuffix.IsEmpty();
	const bool bHadExperienceSuffix = !ExperienceSuffix.IsEmpty();
	if (bHadExperienceSuffix && bHadUserSuffix)
		ComposedNamesToFind.Add(
			FString::Printf(TEXT("%s_%s_%s"), *BasePlatformName, *EffectiveUserSuffix, *ExperienceSuffix));
	if (bHadUserSuffix)
		ComposedNamesToFind.Add(FString::Printf(TEXT("%s_%s"), *BasePlatformName, *EffectiveUserSuffix));
	if (bHadExperienceSuffix)
		ComposedNamesToFind.Add(FString::Printf(TEXT("%s_%s"), *BasePlatformName, *ExperienceSuffix));
	if (GIsEditor)
		ComposedNamesToFind.Add(BasePlatformName);
	return ComposedNamesToFind;
}

FString ULyraSettingsLocal::FindActualProfileToApply(UDeviceProfileManager& Manager,
                                                     const TArray<FString>& ComposedNamesToFind,
                                                     const FName& PlatformName) const
{
	FString ActualProfileToApply;
	for (const FString& TestProfileName : ComposedNamesToFind)
	{
		if (!Manager.HasLoadableProfileName(TestProfileName, PlatformName))
			continue;

		ActualProfileToApply = TestProfileName;
		const UDeviceProfile* Profile = Manager.FindProfile(TestProfileName, /*bCreateOnFail=*/ false);
		if (!Profile)
			Profile = Manager.CreateProfile(TestProfileName, TEXT(""), TestProfileName, *PlatformName.ToString());

		UE_LOG(LogConsoleResponse, Log, TEXT("Profile %s exists"), *Profile->GetName());
		break;
	}
	return ActualProfileToApply;
}


void ULyraSettingsLocal::ApplyDeviceProfileIfNeeded(UDeviceProfileManager& Manager, const FString& ActualProfileToApply)
{
	if (ActualProfileToApply != CurrentAppliedDeviceProfileOverrideSuffix)
	{
		if (Manager.GetActiveDeviceProfileName() != ActualProfileToApply)
		{
			RestoreDefaultDeviceProfile(Manager);
			ApplyNewDeviceProfile(Manager, ActualProfileToApply);
		}
		CurrentAppliedDeviceProfileOverrideSuffix = ActualProfileToApply;
	}
}

void ULyraSettingsLocal::RestoreDefaultDeviceProfile(UDeviceProfileManager& Manager)
{
#if WITH_EDITOR && ALLOW_OTHER_PLATFORM_CONFIG
	if (GIsEditor)
	{
		Manager.RestorePreviewDeviceProfile();
		return;
	}
#endif
	Manager.RestoreDefaultDeviceProfile();
}

void ULyraSettingsLocal::ApplyNewDeviceProfile(UDeviceProfileManager& Manager, const FString& ActualProfileToApply)
{
	UDeviceProfile* NewDeviceProfile = Manager.FindProfile(ActualProfileToApply);
	ensureMsgf(NewDeviceProfile != nullptr, TEXT("DeviceProfile %s not found "), *ActualProfileToApply);
	if (NewDeviceProfile)
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
#if ALLOW_OTHER_PLATFORM_CONFIG
			UE_LOG(LogConsoleResponse, Log, TEXT("Overriding *preview* device profile to %s"), *ActualProfileToApply);
			Manager.SetPreviewDeviceProfile(NewDeviceProfile);
			LyraSettingsHelpers::FillScalabilitySettingsFromDeviceProfile(DeviceDefaultScalabilitySettings);
#endif
		}
		else
#endif
		{
			UE_LOG(LogConsoleResponse, Log, TEXT("Overriding device profile to %s"), *ActualProfileToApply);
			Manager.SetOverrideDeviceProfile(NewDeviceProfile);
		}
	}
}

void ULyraSettingsLocal::UpdateFramePacing(const ELyraFramePacingMode FramePacingMode)
{
	switch (FramePacingMode)
	{
	case ELyraFramePacingMode::MobileStyle:
		UpdateMobileFramePacing();
		break;
	case ELyraFramePacingMode::ConsoleStyle:
		UpdateConsoleFramePacing();
		break;
	case ELyraFramePacingMode::DesktopStyle:
		UpdateDesktopFramePacing();
		break;
	}
}

// void ULyraSettingsLocal::UpdateGameModeDeviceProfileAndFps()
// {
// #if WITH_EDITOR
// 	if (GIsEditor && !CVarApplyDeviceProfilesInPIE.GetValueOnGameThread())
// 	{
// 		return;
// 	}
// #endif
//
// 	UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
//
// 	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
// 	const TArray<FLyraQualityDeviceProfileVariant>& UserFacingVariants = PlatformSettings->
// 		UserFacingDeviceProfileOptions;
//
// 	//@TODO: Might want to allow specific experiences to specify a suffix to attempt to use as well
// 	// The code below will handle searching with this suffix (alone or in conjunction with the frame rate), but nothing sets it right now
// 	FString ExperienceSuffix;
//
// 	// Make sure the chosen setting is supported for the current display, walking down the list to try fallbacks
// 	const int32 PlatformMaxRefreshRate = FPlatformMisc::GetMaxRefreshRate();
//
// 	int32 SuffixIndex = UserFacingVariants.IndexOfByPredicate([&](const FLyraQualityDeviceProfileVariant& Data)
// 	{
// 		return Data.DeviceProfileSuffix == UserChosenDeviceProfileSuffix;
// 	});
// 	while (UserFacingVariants.IsValidIndex(SuffixIndex))
// 	{
// 		if (PlatformMaxRefreshRate >= UserFacingVariants[SuffixIndex].MinRefreshRate)
// 		{
// 			break;
// 		}
// 		else
// 		{
// 			--SuffixIndex;
// 		}
// 	}
//
// 	const FString EffectiveUserSuffix = UserFacingVariants.IsValidIndex(SuffixIndex)
// 		                                    ? UserFacingVariants[SuffixIndex].DeviceProfileSuffix
// 		                                    : PlatformSettings->DefaultDeviceProfileSuffix;
//
//
// 	FString BasePlatformName = UDeviceProfileManager::GetPlatformDeviceProfileName();
// 	FName PlatformName; // Default unless in editor
// #if WITH_EDITOR
// 	if (GIsEditor)
// 	{
// 		const auto Settings = GetDefault<UPlatformEmulationSettings>();
// 		const FName PretendBaseDeviceProfile = Settings->GetPretendBaseDeviceProfile();
// 		if (PretendBaseDeviceProfile != NAME_None)
// 		{
// 			BasePlatformName = PretendBaseDeviceProfile.ToString();
// 		}
//
// 		PlatformName = Settings->GetPretendPlatformName();
// 	}
// #endif
//
// 	TArray<FString> ComposedNamesToFind;
//
// 	// Build up a list of names to try
// 	const bool bHadUserSuffix = !EffectiveUserSuffix.IsEmpty();
// 	const bool bHadExperienceSuffix = !ExperienceSuffix.IsEmpty();
// 	if (bHadExperienceSuffix && bHadUserSuffix)
// 		ComposedNamesToFind.Add(TEXT("%s_%s_%s", *BasePlatformName, *EffectiveUserSuffix, *EffectiveUserSuffix));
// 	if (bHadUserSuffix)
// 		ComposedNamesToFind.Add(+TEXT("%s_%s", *BasePlatformName, *EffectiveUserSuffix));
// 	if (bHadExperienceSuffix)
// 		ComposedNamesToFind.Add(+TEXT("%s_%s", *BasePlatformName, *ExperienceSuffix));
// 	if (GIsEditor)
// 		ComposedNamesToFind.Add(BasePlatformName);
//
// 	// See if any of the potential device profiles actually exists
// 	FString ActualProfileToApply;
// 	for (const FString& TestProfileName : ComposedNamesToFind)
// 	{
// 		if (Manager.HasLoadableProfileName(TestProfileName, PlatformName))
// 		{
// 			ActualProfileToApply = TestProfileName;
// 			UDeviceProfile* Profile = Manager.FindProfile(TestProfileName, /*bCreateOnFail=*/ false);
// 			if (Profile)
// 				Profile = Manager.CreateProfile(TestProfileName, TEXT(""), TestProfileName, *PlatformName.ToString());
// 			
// 			UE_LOG(LogConsoleResponse, Log, TEXT("Profile %s exists"), *Profile->GetName());
// 			break;
// 		}
// 	}
//
// 	UE_LOG(LogConsoleResponse, Log,
// 	       TEXT(
// 		       "UpdateGameModeDeviceProfileAndFps MaxRefreshRate=%d, ExperienceSuffix='%s', UserPicked='%s'->'%s', PlatformBase='%s', AppliedActual='%s'"
// 	       ),
// 	       PlatformMaxRefreshRate, *ExperienceSuffix, *UserChosenDeviceProfileSuffix, *EffectiveUserSuffix,
// 	       *BasePlatformName, *ActualProfileToApply);
//
// 	// Apply the device profile if it's different to what we currently have
// 	if (ActualProfileToApply != CurrentAppliedDeviceProfileOverrideSuffix)
// 	{
// 		if (Manager.GetActiveDeviceProfileName() != ActualProfileToApply)
// 		{
// 			// Restore the default first
// 			if (GIsEditor)
// 			{
// #if ALLOW_OTHER_PLATFORM_CONFIG
// 				Manager.RestorePreviewDeviceProfile();
// #endif
// 			}
// 			else
// 			{
// 				Manager.RestoreDefaultDeviceProfile();
// 			}
//
// 			// Apply the new one (if it wasn't the default)
// 			if (Manager.GetActiveDeviceProfileName() != ActualProfileToApply)
// 			{
// 				UDeviceProfile* NewDeviceProfile = Manager.FindProfile(ActualProfileToApply);
// 				ensureMsgf(NewDeviceProfile != nullptr, TEXT("DeviceProfile %s not found "), *ActualProfileToApply);
// 				if (NewDeviceProfile)
// 				{
// 					if (GIsEditor)
// 					{
// #if ALLOW_OTHER_PLATFORM_CONFIG
// 						UE_LOG(LogConsoleResponse, Log, TEXT("Overriding *preview* device profile to %s"),
// 						       *ActualProfileToApply);
// 						Manager.SetPreviewDeviceProfile(NewDeviceProfile);
//
// 						// Reload the default settings from the pretend profile
// 						LyraSettingsHelpers::FillScalabilitySettingsFromDeviceProfile(DeviceDefaultScalabilitySettings);
// #endif
// 					}
// 					else
// 					{
// 						UE_LOG(LogConsoleResponse, Log, TEXT("Overriding device profile to %s"), *ActualProfileToApply);
// 						Manager.SetOverrideDeviceProfile(NewDeviceProfile);
// 					}
// 				}
// 			}
// 		}
// 		CurrentAppliedDeviceProfileOverrideSuffix = ActualProfileToApply;
// 	}
//
// 	switch (PlatformSettings->FramePacingMode)
// 	{
// 	case ELyraFramePacingMode::MobileStyle:
// 		UpdateMobileFramePacing();
// 		break;
// 	case ELyraFramePacingMode::ConsoleStyle:
// 		UpdateConsoleFramePacing();
// 		break;
// 	case ELyraFramePacingMode::DesktopStyle:
// 		UpdateDesktopFramePacing();
// 		break;
// 	}
// }

void ULyraSettingsLocal::UpdateConsoleFramePacing() const
{
	ApplyFrameSyncType();
	ApplyTargetFPS();
}

void ULyraSettingsLocal::ApplyFrameSyncType() const
{
	const int32 FrameSyncType = CVarDeviceProfileDrivenFrameSyncType.GetValueOnGameThread();
	if (FrameSyncType == -1)return;

	UE_LOG(LogConsoleResponse, Log, TEXT("Setting frame sync mode to %d."), FrameSyncType);
	SetSyncTypeCVar(FrameSyncType);
}

void ULyraSettingsLocal::ApplyTargetFPS() const
{
	const int32 TargetFPS = CVarDeviceProfileDrivenTargetFps.GetValueOnGameThread();
	if (TargetFPS == -1)return;

	UE_LOG(LogConsoleResponse, Log, TEXT("Setting frame pace to %d Hz."), TargetFPS);
	FPlatformRHIFramePacer::SetFramePace(TargetFPS);
	// Set the CSV metadata and analytics Fps mode strings
#if CSV_PROFILER
	const FString TargetFramerateString = FString::Printf(TEXT("%d"), TargetFPS);
	CSV_METADATA(TEXT("TargetFramerate"), *TargetFramerateString);
#endif
}

void ULyraSettingsLocal::UpdateDesktopFramePacing()
{
	// For desktop the frame rate limit is handled by the parent class based on the value already
	// applied via UpdateEffectiveFrameRateLimit()
	// So this function is only doing 'second order' effects of desktop frame pacing preferences

	const float TargetFPS = GetEffectiveFrameRateLimit();
	const float ClampedFPS = (TargetFPS <= 0.0f) ? 60.0f : FMath::Clamp(TargetFPS, 30.0f, 60.0f);
	UpdateDynamicResFrameTime(ClampedFPS);
}

void ULyraSettingsLocal::UpdateMobileFramePacing()
{
	//@TODO: Handle different limits for in-front-end or low-battery mode on mobile

	// Choose the closest supported frame rate to the user desired setting without going over the device imposed limit
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	const TArray<int32>& PossibleRates = PlatformSettings->MobileFrameRateLimits;
	const int32 LimitIndex = PossibleRates.FindLastByPredicate([this](const int32& TestRate)
	{
		return (TestRate <= MobileFrameRateLimit) && IsSupportedMobileFramePace(TestRate);
	});
	const int32 TargetFPS = PossibleRates.IsValidIndex(LimitIndex)
		                        ? PossibleRates[LimitIndex]
		                        : GetDefaultMobileFrameRate();

	UE_LOG(LogConsoleResponse, Log, TEXT("Setting frame pace to %d Hz."), TargetFPS);
	FPlatformRHIFramePacer::SetFramePace(TargetFPS);

	ClampMobileQuality();

	UpdateDynamicResFrameTime(static_cast<float>(TargetFPS));
}

void ULyraSettingsLocal::UpdateDynamicResFrameTime(const float TargetFPS) const
{
	static IConsoleVariable* CVarDyResFrameTimeBudget = IConsoleManager::Get().FindConsoleVariable(
		TEXT("r.DynamicRes.FrameTimeBudget"));
	if (!CVarDyResFrameTimeBudget || !(ensure(TargetFPS > 0.0f))) return;

	const float DyResFrameTimeBudget = 1000.0f / TargetFPS;
	CVarDyResFrameTimeBudget->Set(DyResFrameTimeBudget, ECVF_SetByGameSetting);
}
