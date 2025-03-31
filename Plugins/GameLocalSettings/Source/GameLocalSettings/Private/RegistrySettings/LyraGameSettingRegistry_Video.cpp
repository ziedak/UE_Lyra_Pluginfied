// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSettings/LyraSettingAction_SafeZoneEditor.h"
#include "CustomSettings/LyraSettingValueDiscrete_MobileFPSType.h"
#include "CustomSettings/LyraSettingValueDiscrete_OverallQuality.h"
#include "CustomSettings/LyraSettingValueDiscrete_Resolution.h"
// #include "DataSource/GameSettingDataSource.h"
#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "RegistrySettings/LyraGameSettingRegistry.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
#include "NativeGameplayTags.h"
#include "Performance/LyraPerformanceSettings.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"

#include "Settings/MyClass.h"
#define LOCTEXT_NAMESPACE "Lyra"

UE_DEFINE_GAMEPLAY_TAG_STATIC(GameSettings_Action_EditSafeZone, "GameSettings.Action.EditSafeZone");
UE_DEFINE_GAMEPLAY_TAG_STATIC(GameSettings_Action_EditBrightness, "GameSettings.Action.EditBrightness");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_SupportsWindowedMode, "Platform.Trait.SupportsWindowedMode");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_NeedsBrightnessAdjustment, "Platform.Trait.NeedsBrightnessAdjustment");

//////////////////////////////////////////////////////////////////////

enum class EFramePacingEditCondition
{
	EnableIf,
	DisableIf
};

// Checks the platform-specific value for FramePacingMode
class FGameSettingEditCondition_FramePacingMode : public FGameSettingEditCondition
{
public:
	FGameSettingEditCondition_FramePacingMode(const ELyraFramePacingMode InDesiredMode,
	                                          const EFramePacingEditCondition InMatchMode =
		                                          EFramePacingEditCondition::EnableIf)
		: DesiredMode(InDesiredMode)
		  , MatchMode(InMatchMode) {}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override
	{
		const ELyraFramePacingMode ActualMode = ULyraPlatformSpecificRenderingSettings::Get()->FramePacingMode;

		const bool bMatches = (ActualMode == DesiredMode);
		const bool bMatchesAreBad = (MatchMode == EFramePacingEditCondition::DisableIf);

		if (bMatches == bMatchesAreBad)
		{
			InOutEditState.Kill(FString::Printf(
				TEXT("Frame pacing mode %d didn't match requirement %d"), static_cast<int32>(ActualMode),
				static_cast<int32>(DesiredMode)));
		}
	}

private:
	ELyraFramePacingMode DesiredMode;
	EFramePacingEditCondition MatchMode;
};

//////////////////////////////////////////////////////////////////////

// Checks the platform-specific value for bSupportsGranularVideoQualitySettings
class FGameSettingEditCondition_VideoQuality : public FGameSettingEditCondition
{
public:
	FGameSettingEditCondition_VideoQuality(const FString& InDisableString)
		: DisableString(InDisableString) {}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override
	{
		if (!ULyraPlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings) InOutEditState.Kill(DisableString);
	}

	virtual void SettingChanged(const ULocalPlayer* LocalPlayer, UGameSetting* Setting,
	                            EGameSettingChangeReason Reason) const override
	{
		// TODO for now this applies the setting immediately
		if (!LocalPlayer || !LocalPlayer->Implements<UPlayerSharedSettingsInterface>())
		{
			UE_LOG(LogLyraGameSettingRegistry, Error,
			       TEXT("OwningLocalPlayer does not Implement IPlayerSharedSettings"));
			return;
		}

		if (const auto ISharedSettings = CastChecked<IPlayerSharedSettingsInterface>(LocalPlayer)) ISharedSettings->GetLocalSettings()->ApplyScalabilitySettings();
		// const ULyraLocalPlayer* LyraLocalPlayer = CastChecked<ULyraLocalPlayer>(LocalPlayer);
		// LyraLocalPlayer->GetLocalSettings()->ApplyScalabilitySettings();

		// const UMyClass* LyraLocalPlayer = CastChecked<UMyClass>(LocalPlayer);
		// LyraLocalPlayer->GetLocalSettings()->ApplyScalabilitySettings();
	}

private:
	FString DisableString;
};

////////////////////////////////////////////////////////////////////////////////////


UGameSettingCollection* ULyraGameSettingRegistry::InitializeVideoSettings(ULocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = UGameSettingCollection::CreateCollection(
		"VideoCollection", LOCTEXT("VideoCollection_Name", "Video"));
	Screen->Initialize(InLocalPlayer);

	AddDisplaySettings(Screen);
	AddGraphicsSettings(Screen);
	// AddGraphicsQualitySettings(Screen);
	// AddAdvancedGraphicsSettings(Screen);

	return Screen;
}


void ULyraGameSettingRegistry::AddDisplaySettings(UGameSettingCollection* Screen)
{
	const auto Display = UGameSettingCollection::CreateCollection("DisplayCollection",
	                                                              LOCTEXT("DisplayCollection_Name", "Display"));
	Screen->AddSetting(Display);

	UGameSettingValueDiscreteDynamic_Enum* WindowModeSetting = CreateWindowModeSetting();
	Display->AddSetting(WindowModeSetting);
	Display->AddSetting(CreateResolutionSetting(WindowModeSetting));
	AddPerformanceStatPage(Display);
}

UGameSettingValueDiscreteDynamic_Enum* ULyraGameSettingRegistry::CreateWindowModeSetting()
{
	UGameSettingValueDiscreteDynamic_Enum* Setting = UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings(
		"WindowMode",
		LOCTEXT("WindowMode_Name", "Window Mode"),
		LOCTEXT("WindowMode_Description",
		        "In Windowed mode you can interact with other windows more easily, "
		        "and drag the edges of the window to set the size. In Windowed Fullscreen "
		        "mode you can easily switch between applications. In Fullscreen mode"
		        " you cannot interact with other windows as easily, but the game will run slightly faster."),

		GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFullscreenMode),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFullscreenMode),
		EWindowMode::Windowed,
		{
			{EWindowMode::Fullscreen, LOCTEXT("WindowModeFullscreen", "Fullscreen")},
			{EWindowMode::WindowedFullscreen, LOCTEXT("WindowModeWindowedFullscreen", "Windowed Fullscreen")},
			{EWindowMode::Windowed, LOCTEXT("WindowModeWindowed", "Windowed")}
		}
	);

	Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
			TAG_Platform_Trait_SupportsWindowedMode, TEXT("Platform does not support window mode"))
	);


	return Setting;
}

ULyraSettingValueDiscrete_Resolution* ULyraGameSettingRegistry::CreateResolutionSetting(UGameSettingValueDiscreteDynamic_Enum* EditDependency)
{
	const auto Setting = NewObject<ULyraSettingValueDiscrete_Resolution>();
	Setting->SetDevName(TEXT("Resolution"));
	Setting->SetDisplayName(LOCTEXT("Resolution_Name", "Resolution"));
	Setting->SetDescriptionRichText(LOCTEXT("Resolution_Description",
	                                        "Display Resolution determines the size of the window in Windowed mode. In Fullscreen mode, Display Resolution determines the graphics card output resolution, which can result in black bars depending on monitor and graphics card. Display Resolution is inactive in Windowed Fullscreen mode."));

	Setting->AddEditDependency(EditDependency);
	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	return Setting;
}

void ULyraGameSettingRegistry::AddGraphicsSettings(UGameSettingCollection* Screen)
{
	const auto Graphics = UGameSettingCollection::CreateCollection("GraphicsCollection",
	                                                               LOCTEXT("GraphicsCollection_Name", "Graphics"));
	Screen->AddSetting(Graphics);

	Graphics->AddSetting(AddColorBlindModeSetting());
	Graphics->AddSetting(AddBrightnessSetting());
	Graphics->AddSetting(AddSafeZoneSetting());
}

UGameSettingValueDiscreteDynamic_Enum* ULyraGameSettingRegistry::AddColorBlindModeSetting()
{
	const auto Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
	Setting->SetDevName(TEXT("ColorBlindMode"));
	Setting->SetDisplayName(LOCTEXT("ColorBlindMode_Name", "Color Blind Mode"));
	Setting->SetDescriptionRichText(LOCTEXT("ColorBlindMode_Description",
	                                        "Using the provided images, test out the different color blind modes to find a color correction that works best for you."));

	Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindMode));
	Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindMode));
	Setting->SetDefaultValue(GetDefault<ULyraSettingsShared>()->GetColorBlindMode());
	Setting->AddEnumOption(EColorBlindMode::Off, LOCTEXT("ColorBlindRotatorSettingOff", "Off"));
	Setting->AddEnumOption(EColorBlindMode::Deuteranope, LOCTEXT("ColorBlindRotatorSettingDeuteranope", "Deuteranope"));
	Setting->AddEnumOption(EColorBlindMode::Protanope, LOCTEXT("ColorBlindRotatorSettingProtanope", "Protanope"));
	Setting->AddEnumOption(EColorBlindMode::Tritanope, LOCTEXT("ColorBlindRotatorSettingTritanope", "Tritanope"));

	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	return Setting;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::AddBrightnessSetting()
{
	const auto Setting = NewObject<UGameSettingValueScalarDynamic>();
	Setting->SetDevName(TEXT("Brightness"));
	Setting->SetDisplayName(LOCTEXT("Brightness_Name", "Brightness"));
	Setting->SetDescriptionRichText(LOCTEXT("Brightness_Description", "Adjusts the brightness."));

	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDisplayGamma));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDisplayGamma));
	Setting->SetDefaultValue(2.2);
	Setting->SetDisplayFormat([](double SourceValue, const double NormalizedValue){
		return FText::Format(
			LOCTEXT("BrightnessFormat", "{0}%"),
			static_cast<int32>(FMath::GetMappedRangeValueClamped(
				FVector2D(0, 1), FVector2D(50, 150), NormalizedValue)));
	});
	Setting->SetSourceRangeAndStep(TRange<double>(1.7, 2.7), 0.01);

	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
		TAG_Platform_Trait_NeedsBrightnessAdjustment,
		TEXT("Platform does not require brightness adjustment.")));

	return Setting;
}

ULyraSettingAction_SafeZoneEditor* ULyraGameSettingRegistry::AddSafeZoneSetting()
{
	const auto Setting = NewObject<ULyraSettingAction_SafeZoneEditor>();
	Setting->SetDevName(TEXT("SafeZone"));
	Setting->SetDisplayName(LOCTEXT("SafeZone_Name", "Safe Zone"));
	Setting->SetDescriptionRichText(LOCTEXT("SafeZone_Description", "Set the UI safe zone for the platform."));
	Setting->SetActionText(LOCTEXT("SafeZone_Action", "Set Safe Zone"));
	Setting->SetNamedAction(GameSettings_Action_EditSafeZone);

	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	Setting->AddEditCondition(MakeShared<FWhenCondition>(
		[](const ULocalPlayer*, FGameSettingEditableState& InOutEditState){
			FDisplayMetrics Metrics;
			FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
			if (Metrics.TitleSafePaddingSize.Size() == 0)
			{
				InOutEditState.Kill(TEXT(
					"Platform does not have any TitleSafePaddingSize configured in the display metrics."));
			}
		}));

	return Setting;
}

// void ULyraGameSettingRegistry::AddGraphicsQualitySettings(UGameSettingCollection* Screen)
// {
// 	// Implementation for adding graphics quality settings
// }
//
// void ULyraGameSettingRegistry::AddAdvancedGraphicsSettings(UGameSettingCollection* Screen)
// {
// 	// Implementation for adding advanced graphics settings
// }

void AddFrameRateOptions(const auto Setting)
{
	const FText FPSFormat = LOCTEXT("FPSFormat", "{0} FPS");
	for (const int32 Rate : GetDefault<ULyraPerformanceSettings>()->DesktopFrameRateLimits) { Setting->AddOption(static_cast<float>(Rate), FText::Format(FPSFormat, Rate)); }
	Setting->AddOption(0.0f, LOCTEXT("UnlimitedFPS", "Unlimited"));
}

void ULyraGameSettingRegistry::InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen,
                                                                  ULocalPlayer* InLocalPlayer)
{
	struct FrameRateSettingConfig
	{
		const FName DevName;
		FText DisplayName;
		FText Description;
		const TSharedRef<FGameSettingDataSource> GetterPath;
		const TSharedRef<FGameSettingDataSource> SetterPath;
		float DefaultValue;
	};

	const TArray<FrameRateSettingConfig> FrameRateSettings = {
		{
			TEXT("FrameRateLimit_OnBattery"),
			LOCTEXT("FrameRateLimit_OnBattery_Name", "Frame Rate Limit (On Battery)"),
			LOCTEXT("FrameRateLimit_OnBattery_Description",
			        "Frame rate limit when running on battery. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_OnBattery),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_OnBattery),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_OnBattery()
		},
		{
			TEXT("FrameRateLimit_InMenu"),
			LOCTEXT("FrameRateLimit_InMenu_Name", "Frame Rate Limit (Menu)"),
			LOCTEXT("FrameRateLimit_InMenu_Description",
			        "Frame rate limit when in the menu. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_InMenu),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_InMenu),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_InMenu()
		},
		{
			TEXT("FrameRateLimit_WhenBackgrounded"),
			LOCTEXT("FrameRateLimit_WhenBackgrounded_Name", "Frame Rate Limit (Background)"),
			LOCTEXT("FrameRateLimit_WhenBackgrounded_Description",
			        "Frame rate limit when in the background. Set this lower for a more consistent frame rate or higher "
			        "for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_WhenBackgrounded),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_WhenBackgrounded),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_WhenBackgrounded()
		},
		{
			TEXT("FrameRateLimit_Always"),
			LOCTEXT("FrameRateLimit_Always_Name", "Frame Rate Limit"),
			LOCTEXT("FrameRateLimit_Always_Description",
			        "Frame rate limit sets the highest frame rate that is allowed. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_Always),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_Always),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_Always()
		}
	};

	for (const auto& Config : FrameRateSettings)
	{
		const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
			Config.DevName,
			Config.DisplayName,
			Config.Description,
			Config.GetterPath,
			Config.SetterPath,
			Config.DefaultValue,
			1);

		Setting->AddEditCondition(
			MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
}

#undef LOCTEXT_NAMESPACE