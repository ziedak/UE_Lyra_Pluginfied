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

#include "CustomSettings/EditConditions/WhenFramePacingModeMatch.h"
#include "CustomSettings/EditConditions/WhenSupportsGranularVideoQuality.h"
#include "CustomSettings/EditConditions/WhenAutoQualitySupported.h"

#include "RegistrySettings/LyraGameSettingRegistry.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
#include "Settings/SettingsTags.h"
#include "Performance/LyraPerformanceSettings.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"

#define LOCTEXT_NAMESPACE "Lyra"


UGameSettingCollection* ULyraGameSettingRegistry::InitializeVideoSettings(ULocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = UGameSettingCollection::CreateCollection(
		"VideoCollection", LOCTEXT("VideoCollection_Name", "Video"));
	Screen->Initialize(InLocalPlayer);

	AddDisplaySettings(Screen);
	AddGraphicsSettings(Screen);
	AddGraphicsQualitySettings(Screen);
	AddAdvancedGraphicsSettings(Screen);

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
	Display->AddSetting(CreateVerticalSyncSettings(WindowModeSetting));
	AddPerformanceStatPage(Display);
}

UGameSettingValueDiscreteDynamic_Enum* ULyraGameSettingRegistry::CreateWindowModeSetting()
{
	UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
	Setting->SetDevName(TEXT("WindowMode"));
	Setting->SetDisplayName(LOCTEXT("WindowMode_Name", "Window Mode"));
	Setting->SetDescriptionRichText(LOCTEXT("WindowMode_Description",
	                                        "In Windowed mode you can interact with other windows more easily, and drag the edges of the window to set the size. In Windowed Fullscreen mode you can easily switch between applications. In Fullscreen mode you cannot interact with other windows as easily, but the game will run slightly faster."));

	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFullscreenMode));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFullscreenMode));
	Setting->AddEnumOption(EWindowMode::Fullscreen, LOCTEXT("WindowModeFullscreen", "Fullscreen"));
	Setting->AddEnumOption(EWindowMode::WindowedFullscreen, LOCTEXT("WindowModeWindowedFullscreen", "Windowed Fullscreen"));
	Setting->AddEnumOption(EWindowMode::Windowed, LOCTEXT("WindowModeWindowed", "Windowed"));

	Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(PlatformTags::TRAIT_SUPPORTS_WINDOWED_MODE, TEXT("Platform does not support window mode")));


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
	Graphics->AddSetting(AddColorBlindStrength());
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

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::AddColorBlindStrength()
{
	const auto Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("ColorBlindStrength"));
	Setting->SetDisplayName(LOCTEXT("ColorBlindStrength_Name", "Color Blind Strength"));
	Setting->SetDescriptionRichText(LOCTEXT("ColorBlindStrength_Description", "Using the provided images, test out the different strengths to find a color correction that works best for you."));

	Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindStrength));
	Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindStrength));
	Setting->SetDefaultValue(GetDefault<ULyraSettingsShared>()->GetColorBlindStrength());
	for (int32 Index = 0; Index <= 10; Index++) { Setting->AddOption(Index, FText::AsNumber(Index)); }

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
	Setting->SetDisplayFormat([](double, const double NormalizedValue){
		return FText::Format(
			LOCTEXT("BrightnessFormat", "{0}%"),
			static_cast<int32>(FMath::GetMappedRangeValueClamped(
				FVector2D(0, 1), FVector2D(50, 150), NormalizedValue)));
	});
	Setting->SetSourceRangeAndStep(TRange<double>(1.7, 2.7), 0.01);

	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
		PlatformTags::TRAIT_NEEDS_BRIGHTNESS_ADJUSTMENT,
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
	Setting->SetNamedAction(GameSettingsTags::ACTION_EDIT_SAFEZONE);

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

void ULyraGameSettingRegistry::AddGraphicsQualitySettings(UGameSettingCollection* Screen)
{
	UGameSettingCollection* GraphicsQuality = UGameSettingCollection::CreateCollection(TEXT("GraphicsQuality"), LOCTEXT("GraphicsQuality_Name", "Graphics Quality"));
	Screen->AddSetting(GraphicsQuality);

	const auto AutoSetQuality = CreateAutoSetQualitySetting();
	const auto GraphicsQualityPresets = CreateGraphicsQualityPresetsSetting();
	const auto MobileFPSType = CreateMobileFPSTypeSetting(AutoSetQuality, GraphicsQualityPresets);
	const auto ProfileSuffixSetting = CreateDeviceProfileSuffixSetting();
	const auto ResolutionScale = CreateResolutionScaleSetting(AutoSetQuality, GraphicsQualityPresets);
	const auto GlobalIlluminationQuality = CreateGlobalIlluminationQualitySetting(AutoSetQuality, GraphicsQualityPresets);
	const auto Shadows = CreateShadowsSetting(AutoSetQuality, GraphicsQualityPresets);
	const auto AntiAliasing = CreateAntiAliasingSetting(AutoSetQuality, GraphicsQualityPresets);
	const auto ViewDistance = CreateViewDistanceSetting(AutoSetQuality, GraphicsQualityPresets);
	const auto TextureQuality = CreateTextureQualitySetting(AutoSetQuality, GraphicsQualityPresets);
	const auto VisualEffectQuality = CreateVisualEffectQualitySetting(AutoSetQuality, GraphicsQualityPresets);
	const auto ReflectionQuality = CreateReflectionQualitySetting(AutoSetQuality, GraphicsQualityPresets);
	const auto PostProcessingQuality = CreatePostProcessingQualitySetting(AutoSetQuality, GraphicsQualityPresets);

	if (ProfileSuffixSetting->GetDynamicOptions().Num() > 1) GraphicsQuality->AddSetting(ProfileSuffixSetting);

	if (MobileFPSType)
	{
		GraphicsQualityPresets->AddEditDependency(MobileFPSType);
		GraphicsQuality->AddSetting(MobileFPSType);
	}

	const TArray<UGameSetting*> EditDependencies =
	{
		AutoSetQuality,
		ResolutionScale,
		GlobalIlluminationQuality,
		Shadows,
		AntiAliasing,
		ViewDistance,
		TextureQuality,
		VisualEffectQuality,
		ReflectionQuality,
		PostProcessingQuality
	};
	for (const auto Dependency : EditDependencies) { GraphicsQualityPresets->AddEditDependency(Dependency); }

	GraphicsQuality->AddSetting(AutoSetQuality);
	GraphicsQuality->AddSetting(GraphicsQualityPresets);
	GraphicsQuality->AddSetting(ResolutionScale);
	GraphicsQuality->AddSetting(GlobalIlluminationQuality);
	GraphicsQuality->AddSetting(Shadows);
	GraphicsQuality->AddSetting(AntiAliasing);
	GraphicsQuality->AddSetting(ViewDistance);
	GraphicsQuality->AddSetting(TextureQuality);
	GraphicsQuality->AddSetting(VisualEffectQuality);
	GraphicsQuality->AddSetting(ReflectionQuality);
	GraphicsQuality->AddSetting(PostProcessingQuality);
}

UGameSettingValueDiscreteDynamic* ULyraGameSettingRegistry::CreateDeviceProfileSuffixSetting()
{
	const auto Setting = NewObject<UGameSettingValueDiscreteDynamic>();
	Setting->SetDevName(TEXT("DeviceProfileSuffix"));
	Setting->SetDisplayName(LOCTEXT("DeviceProfileSuffix_Name", "Device Profile"));
	Setting->SetDescriptionRichText(LOCTEXT("DeviceProfileSuffix_Description", "Choose between different quality presets to make a trade off between quality and speed."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDesiredDeviceProfileQualitySuffix));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDesiredDeviceProfileQualitySuffix));

	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	Setting->SetDefaultValueFromString(PlatformSettings->DefaultDeviceProfileSuffix);
	for (const FLyraQualityDeviceProfileVariant& Variant : PlatformSettings->UserFacingDeviceProfileOptions)
	{
		if (FPlatformMisc::GetMaxRefreshRate() >= Variant.MinRefreshRate) Setting->AddDynamicOption(Variant.DeviceProfileSuffix, Variant.DisplayName);
	}

	return Setting;
}

ULyraSettingValueDiscrete_MobileFPSType* ULyraGameSettingRegistry::CreateMobileFPSTypeSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	ULyraSettingValueDiscrete_MobileFPSType* Setting = NewObject<ULyraSettingValueDiscrete_MobileFPSType>();
	Setting->SetDevName(TEXT("FrameRateLimit_Mobile"));
	Setting->SetDisplayName(LOCTEXT("FrameRateLimit_Mobile_Name", "Frame Rate Limit Mobile"));
	Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_Mobile_Description", "Select a desired framerate. Use this to fine tune performance on your device."));
	Setting->AddEditCondition(FWhenFramePacingModeMatch::KillIfMatch(EFramePacingMode::MobileStyle));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	return Setting;
}

UGameSettingAction* ULyraGameSettingRegistry::CreateAutoSetQualitySetting()
{
	UGameSettingAction* Setting = NewObject<UGameSettingAction>();
	Setting->SetDevName(TEXT("AutoSetQuality"));
	Setting->SetDisplayName(LOCTEXT("AutoSetQuality_Name", "Auto-Set Quality"));
	Setting->SetDescriptionRichText(LOCTEXT("AutoSetQuality_Description", "Automatically configure the graphics quality options based on a benchmark of the hardware."));
	Setting->SetDoesActionDirtySettings(true);
	Setting->SetActionText(LOCTEXT("AutoSetQuality_Action", "Auto-Set"));
	Setting->SetCustomAction([](ULocalPlayer* LocalPlayer){
		const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
		if (PlatformSettings->FramePacingMode == EFramePacingMode::MobileStyle) ULyraSettingsLocal::Get()->ResetToMobileDeviceDefaults();
		else
		{
			if (const auto ISharedSettings = CastChecked<IPlayerSharedSettingsInterface>(LocalPlayer))
			{
				constexpr bool bImmediatelySaveState = false;
				ISharedSettings->GetLocalSettings()->RunAutoBenchmark(bImmediatelySaveState);
			}
		}
	});

	Setting->AddEditCondition(FWhenAutoQualitySupported::KillIfNot());

	return Setting;
}

ULyraSettingValueDiscrete_OverallQuality* ULyraGameSettingRegistry::CreateGraphicsQualityPresetsSetting()
{
	ULyraSettingValueDiscrete_OverallQuality* Setting = NewObject<ULyraSettingValueDiscrete_OverallQuality>();
	Setting->SetDevName(TEXT("GraphicsQualityPresets"));
	Setting->SetDisplayName(LOCTEXT("GraphicsQualityPresets_Name", "Quality Presets"));
	Setting->SetDescriptionRichText(LOCTEXT("GraphicsQualityPresets_Description",
	                                        "Quality Preset allows you to adjust multiple video options at once. Try a few options to see what fits your preference and device's performance."));

	Setting->AddEditCondition(FWhenFramePacingModeMatch::KillIfMatch(EFramePacingMode::ConsoleStyle, EFramePacingEditCondition::DisableIf));

	return Setting;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::CreateResolutionScaleSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
	Setting->SetDevName(TEXT("ResolutionScale"));
	Setting->SetDisplayName(LOCTEXT("ResolutionScale_Name", "3D Resolution"));
	Setting->SetDescriptionRichText(LOCTEXT("ResolutionScale_Description",
	                                        "3D resolution determines the resolution that objects are rendered in game, but does not affect the main menu. Lower resolutions can significantly increase frame rate."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetResolutionScaleNormalized));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetResolutionScaleNormalized));
	Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support 3D Resolution")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreateGlobalIlluminationQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("GlobalIlluminationQuality"));
	Setting->SetDisplayName(LOCTEXT("GlobalIlluminationQuality_Name", "Global Illumination"));
	Setting->SetDescriptionRichText(LOCTEXT("GlobalIlluminationQuality_Description",
	                                        "Global Illumination controls the quality of dynamically calculated indirect lighting bounces, sky shadowing and Ambient Occlusion. Settings of 'High' and above use more accurate ray tracing methods to solve lighting, but can reduce performance."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetGlobalIlluminationQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetGlobalIlluminationQuality));
	Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
	Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
	Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support Global Illumination Quality")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreateShadowsSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("Shadows"));
	Setting->SetDisplayName(LOCTEXT("Shadows_Name", "Shadows"));
	Setting->SetDescriptionRichText(LOCTEXT("Shadows_Description",
	                                        "Shadow quality determines the resolution and view distance of dynamic shadows. Shadows improve visual quality and give better depth perception, but can reduce performance."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetShadowQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetShadowQuality));
	Setting->AddOption(0, LOCTEXT("ShadowLow", "Off"));
	Setting->AddOption(1, LOCTEXT("ShadowMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("ShadowHigh", "High"));
	Setting->AddOption(3, LOCTEXT("ShadowEpic", "Epic"));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support Shadows")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreateAntiAliasingSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("AntiAliasing"));
	Setting->SetDisplayName(LOCTEXT("AntiAliasing_Name", "Anti-Aliasing"));
	Setting->SetDescriptionRichText(LOCTEXT("AntiAliasing_Description",
	                                        "Anti-Aliasing reduces jaggy artifacts along geometry edges. Increasing this setting will make edges look smoother, but can reduce performance. Higher settings mean more anti-aliasing."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetAntiAliasingQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetAntiAliasingQuality));
	Setting->AddOption(0, LOCTEXT("AntiAliasingLow", "Off"));
	Setting->AddOption(1, LOCTEXT("AntiAliasingMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("AntiAliasingHigh", "High"));
	Setting->AddOption(3, LOCTEXT("AntiAliasingEpic", "Epic"));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support Anti-Aliasing")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreateViewDistanceSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("ViewDistance"));
	Setting->SetDisplayName(LOCTEXT("ViewDistance_Name", "View Distance"));
	Setting->SetDescriptionRichText(LOCTEXT("ViewDistance_Description", "View distance determines how far away objects are culled for performance."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetViewDistanceQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetViewDistanceQuality));
	Setting->AddOption(0, LOCTEXT("ViewDistanceNear", "Near"));
	Setting->AddOption(1, LOCTEXT("ViewDistanceMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("ViewDistanceFar", "Far"));
	Setting->AddOption(3, LOCTEXT("ViewDistanceEpic", "Epic"));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support View Distance")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreateTextureQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("TextureQuality"));
	Setting->SetDisplayName(LOCTEXT("TextureQuality_Name", "Textures"));
	Setting->SetDescriptionRichText(LOCTEXT("TextureQuality_Description",
	                                        "Texture quality determines the resolution of textures in game. Increasing this setting will make objects more detailed, but can reduce performance."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetTextureQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetTextureQuality));
	Setting->AddOption(0, LOCTEXT("TextureQualityLow", "Low"));
	Setting->AddOption(1, LOCTEXT("TextureQualityMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("TextureQualityHigh", "High"));
	Setting->AddOption(3, LOCTEXT("TextureQualityEpic", "Epic"));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support Texture quality")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreateVisualEffectQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("VisualEffectQuality"));
	Setting->SetDisplayName(LOCTEXT("VisualEffectQuality_Name", "Effects"));
	Setting->SetDescriptionRichText(LOCTEXT("VisualEffectQuality_Description",
	                                        "Effects determines the quality of visual effects and lighting in game. Increasing this setting will increase the quality of visual effects, but can reduce performance."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetVisualEffectQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVisualEffectQuality));
	Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
	Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
	Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support Visual Effect Quality")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreateReflectionQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)
{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("ReflectionQuality"));
	Setting->SetDisplayName(LOCTEXT("ReflectionQuality_Name", "Reflections"));
	Setting->SetDescriptionRichText(LOCTEXT("ReflectionQuality_Description",
	                                        "Reflection quality determines the resolution and accuracy of reflections. Settings of 'High' and above use more accurate ray tracing methods to solve reflections, but can reduce performance."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetReflectionQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetReflectionQuality));
	Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
	Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
	Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));
	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support Reflection Quality")));

	return Setting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::CreatePostProcessingQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets)

{
	UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
	Setting->SetDevName(TEXT("PostProcessingQuality"));
	Setting->SetDisplayName(LOCTEXT("PostProcessingQuality_Name", "Post Processing"));
	Setting->SetDescriptionRichText(LOCTEXT("PostProcessingQuality_Description",
	                                        "Post Processing effects include Motion Blur, Depth of Field and Bloom. Increasing this setting improves the quality of post process effects, but can reduce performance."));

	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetPostProcessingQuality));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetPostProcessingQuality));
	Setting->AddOption(0, LOCTEXT("PostProcessingQualityLow", "Low"));
	Setting->AddOption(1, LOCTEXT("PostProcessingQualityMedium", "Medium"));
	Setting->AddOption(2, LOCTEXT("PostProcessingQualityHigh", "High"));
	Setting->AddOption(3, LOCTEXT("PostProcessingQualityEpic", "Epic"));

	Setting->AddEditDependency(AutoSetQuality);
	Setting->AddEditDependency(GraphicsQualityPresets);
	Setting->AddEditCondition(FWhenSupportsGranularVideoQuality::KillIfNotSupported(TEXT("Platform does not support Post Processing Quality")));

	// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
	// GraphicsQualityPresets->AddEditDependency(Setting);

	return Setting;
}

void ULyraGameSettingRegistry::AddAdvancedGraphicsSettings(UGameSettingCollection* Screen)
{
	const auto AdvancedGraphics = UGameSettingCollection::CreateCollection(TEXT("AdvancedGraphics"),LOCTEXT("AdvancedGraphics_Name", "Advanced Graphics"));
	Screen->AddSetting(AdvancedGraphics);

	InitializeVideoSettings_FrameRates(AdvancedGraphics);
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::CreateVerticalSyncSettings(UGameSettingValueDiscreteDynamic_Enum* EditSetting)
{
	const auto Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
	Setting->SetDevName(TEXT("VerticalSync"));
	Setting->SetDisplayName(LOCTEXT("VerticalSync_Name", "Vertical Sync"));
	Setting->SetDescriptionRichText(LOCTEXT("VerticalSync_Description",
	                                        "Enabling Vertical Sync eliminates screen tearing by always rendering and presenting a full frame. Disabling Vertical Sync can give higher frame rate and better input response, but can result in horizontal screen tearing."));

	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(IsVSyncEnabled));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVSyncEnabled));
	Setting->SetDefaultValue(false);

	Setting->AddEditCondition(FWhenFramePacingModeMatch::KillIfMatch(EFramePacingMode::DesktopStyle));

	Setting->AddEditDependency(EditSetting);
	Setting->AddEditCondition(MakeShared<FWhenCondition>([EditSetting](const ULocalPlayer*, FGameSettingEditableState& InOutEditState){
		if (EditSetting->GetValue<EWindowMode::Type>() != EWindowMode::Fullscreen)
		{
			InOutEditState.Disable(
				LOCTEXT("FullscreenNeededForVSync", "This feature only works if 'Window Mode' is set to 'Fullscreen'."));
		}
	}));
	return Setting;
}

void AddFrameRateOptions(const auto Setting)
{
	const FText FPSFormat = LOCTEXT("FPSFormat", "{0} FPS");
	for (const int32 Rate : GetDefault<ULyraPerformanceSettings>()->DesktopFrameRateLimits) { Setting->AddOption(static_cast<float>(Rate), FText::Format(FPSFormat, Rate)); }
	Setting->AddOption(0.0f, LOCTEXT("UnlimitedFPS", "Unlimited"));
}

void ULyraGameSettingRegistry::InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen)
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
			FWhenFramePacingModeMatch::KillIfMatch(EFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
}

#undef LOCTEXT_NAMESPACE