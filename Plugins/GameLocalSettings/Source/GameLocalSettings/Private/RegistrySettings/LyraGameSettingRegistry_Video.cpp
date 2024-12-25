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
// #include "Player/LyraLocalPlayer.h"

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
	FGameSettingEditCondition_FramePacingMode(ELyraFramePacingMode InDesiredMode,
	                                          EFramePacingEditCondition InMatchMode =
		                                          EFramePacingEditCondition::EnableIf)
		: DesiredMode(InDesiredMode)
		  , MatchMode(InMatchMode)
	{
	}

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
		: DisableString(InDisableString)
	{
	}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override
	{
		if (!ULyraPlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings)
			InOutEditState.Kill(DisableString);
	}

	virtual void SettingChanged(const ULocalPlayer* LocalPlayer, UGameSetting* Setting,
	                            EGameSettingChangeReason Reason) const override
	{
		// TODO for now this applies the setting immediately
		if (!LocalPlayer || !LocalPlayer->Implements<UPlayerSharedSettingsInterface>())
			return;

		if (const auto ISharedSettings = CastChecked<IPlayerSharedSettingsInterface>(LocalPlayer))
			ISharedSettings->GetLocalSettings()->ApplyScalabilitySettings();
		// const ULyraLocalPlayer* LyraLocalPlayer = CastChecked<ULyraLocalPlayer>(LocalPlayer);
		// LyraLocalPlayer->GetLocalSettings()->ApplyScalabilitySettings();
	}

private:
	FString DisableString;
};

////////////////////////////////////////////////////////////////////////////////////

UGameSettingCollection* ULyraGameSettingRegistry::InitializeVideoSettings(ULocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = UGameSettingCollection::CreateCollection(
		"VideoCollection",LOCTEXT("VideoCollection_Name", "Video"));
	Screen->Initialize(InLocalPlayer);

	UGameSettingValueDiscreteDynamic_Enum* WindowModeSetting;

	// Display
	////////////////////////////////////////////////////////////////////////////////////
	{
		const auto Display = UGameSettingCollection::CreateCollection("DisplayCollection",
		                                                              LOCTEXT("DisplayCollection_Name", "Display"));
		Screen->AddSetting(Display);

		//----------------------------------------------------------------------------------
		{
			//TODO seet video window default value
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
					{
						EWindowMode::Fullscreen,
						LOCTEXT("WindowModeFullscreen", "Fullscreen")
					},
					{
						EWindowMode::WindowedFullscreen,
						LOCTEXT("WindowModeWindowedFullscreen", "Windowed Fullscreen")
					},
					{
						EWindowMode::Windowed,
						LOCTEXT("WindowModeWindowed", "Windowed")
					}
				}
			);

			Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
					TAG_Platform_Trait_SupportsWindowedMode, TEXT("Platform does not support window mode"))
			);

			WindowModeSetting = Setting;

			Display->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			ULyraSettingValueDiscrete_Resolution* Setting = NewObject<ULyraSettingValueDiscrete_Resolution>();
			Setting->SetDevName(TEXT("Resolution"));
			Setting->SetDisplayName(LOCTEXT("Resolution_Name", "Resolution"));
			Setting->SetDescriptionRichText(LOCTEXT("Resolution_Description",
			                                        "Display Resolution determines the size of the window in Windowed mode. In Fullscreen mode, Display Resolution determines the graphics card output resolution, which can result in black bars depending on monitor and graphics card. Display Resolution is inactive in Windowed Fullscreen mode."));

			Setting->AddEditDependency(WindowModeSetting);
			Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
				TAG_Platform_Trait_SupportsWindowedMode,
				TEXT("Platform does not support window mode")));
			Setting->AddEditCondition(MakeShared<FWhenCondition>(
				[WindowModeSetting](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
				{
					if (WindowModeSetting->GetValue<EWindowMode::Type>() == EWindowMode::WindowedFullscreen)
					{
						InOutEditState.Disable(LOCTEXT("ResolutionWindowedFullscreen_Disabled",
						                               "When the Window Mode is set to <strong>Windowed Fullscreen</>, the resolution must match the native desktop resolution."));
					}
				}));

			Display->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			AddPerformanceStatPage(Display, InLocalPlayer);
		}
		//----------------------------------------------------------------------------------
	}

	// Graphics
	////////////////////////////////////////////////////////////////////////////////////
	{
		const auto Graphics = UGameSettingCollection::CreateCollection("GraphicsCollection",
		                                                               LOCTEXT("GraphicsCollection_Name", "Graphics"));
		Screen->AddSetting(Graphics);

		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("ColorBlindMode"));
			Setting->SetDisplayName(LOCTEXT("ColorBlindMode_Name", "Color Blind Mode"));
			Setting->SetDescriptionRichText(LOCTEXT("ColorBlindMode_Description",
			                                        "Using the provided images, test out the different color blind modes to find a color correction that works best for you."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindMode));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindMode));
			Setting->SetDefaultValue(GetDefault<ULyraSettingsShared>()->GetColorBlindMode());
			Setting->AddEnumOption(EColorBlindMode::Off, LOCTEXT("ColorBlindRotatorSettingOff", "Off"));
			Setting->AddEnumOption(EColorBlindMode::Deuteranope,
			                       LOCTEXT("ColorBlindRotatorSettingDeuteranope", "Deuteranope"));
			Setting->AddEnumOption(EColorBlindMode::Protanope,
			                       LOCTEXT("ColorBlindRotatorSettingProtanope", "Protanope"));
			Setting->AddEnumOption(EColorBlindMode::Tritanope,
			                       LOCTEXT("ColorBlindRotatorSettingTritanope", "Tritanope"));

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Graphics->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("ColorBlindStrength"),
				LOCTEXT("ColorBlindStrength_Name", "Color Blind Strength"),
				LOCTEXT("ColorBlindStrength_Description",
				        "Using the provided images, test out the different strengths to find a color correction that works best for you."),

				GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindStrength),
				GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindStrength),
				GetDefault<ULyraSettingsShared>()->GetColorBlindStrength(), 10);


			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

			Graphics->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("Brightness"));
			Setting->SetDisplayName(LOCTEXT("Brightness_Name", "Brightness"));
			Setting->SetDescriptionRichText(LOCTEXT("Brightness_Description", "Adjusts the brightness."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDisplayGamma));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDisplayGamma));
			Setting->SetDefaultValue(2.2);
			Setting->SetDisplayFormat([](double SourceValue, double NormalizedValue)
			{
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

			Graphics->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			ULyraSettingAction_SafeZoneEditor* Setting = NewObject<ULyraSettingAction_SafeZoneEditor>();
			Setting->SetDevName(TEXT("SafeZone"));
			Setting->SetDisplayName(LOCTEXT("SafeZone_Name", "Safe Zone"));
			Setting->SetDescriptionRichText(LOCTEXT("SafeZone_Description", "Set the UI safe zone for the platform."));
			Setting->SetActionText(LOCTEXT("SafeZone_Action", "Set Safe Zone"));
			Setting->SetNamedAction(GameSettings_Action_EditSafeZone);

			Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
			Setting->AddEditCondition(MakeShared<FWhenCondition>(
				[](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
				{
					FDisplayMetrics Metrics;
					FSlateApplication::Get().GetCachedDisplayMetrics(Metrics);
					if (Metrics.TitleSafePaddingSize.Size() == 0)
					{
						InOutEditState.Kill(TEXT(
							"Platform does not have any TitleSafePaddingSize configured in the display metrics."));
					}
				}));

			Graphics->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
	}


	// Graphics Quality
	////////////////////////////////////////////////////////////////////////////////////
	{
		UGameSetting* MobileFPSType;
		const auto GraphicsQuality = UGameSettingCollection::CreateCollection(
			"GraphicsQuality",LOCTEXT("GraphicsQuality_Name", "Graphics Quality"));
		Screen->AddSetting(GraphicsQuality);

		UGameSetting* AutoSetQuality;
		UGameSetting* GraphicsQualityPresets;

		//----------------------------------------------------------------------------------
		{
			// Console-style device profile selection
			UGameSettingValueDiscreteDynamic* Setting = NewObject<UGameSettingValueDiscreteDynamic>();
			Setting->SetDevName(TEXT("DeviceProfileSuffix"));
			Setting->SetDisplayName(LOCTEXT("DeviceProfileSuffix_Name", "Quality Presets"));
			Setting->SetDescriptionRichText(LOCTEXT("DeviceProfileSuffix_Description",
			                                        "Choose between different quality presets to make a trade off between quality and speed."));
			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDesiredDeviceProfileQualitySuffix));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDesiredDeviceProfileQualitySuffix));

			const ULyraPlatformSpecificRenderingSettings* PlatformSettings =
				ULyraPlatformSpecificRenderingSettings::Get();

			Setting->SetDefaultValueFromString(PlatformSettings->DefaultDeviceProfileSuffix);
			for (const FLyraQualityDeviceProfileVariant& Variant : PlatformSettings->UserFacingDeviceProfileOptions)
			{
				if (FPlatformMisc::GetMaxRefreshRate() >= Variant.MinRefreshRate)
				{
					Setting->AddDynamicOption(Variant.DeviceProfileSuffix, Variant.DisplayName);
				}
			}

			if (Setting->GetDynamicOptions().Num() > 1)
			{
				GraphicsQuality->AddSetting(Setting);
			}
		}

		//----------------------------------------------------------------------------------
		{
			// Mobile style frame rate selection
			ULyraSettingValueDiscrete_MobileFPSType* Setting = NewObject<ULyraSettingValueDiscrete_MobileFPSType>();
			MobileFPSType = Setting;

			Setting->SetDevName(TEXT("FrameRateLimit_Mobile"));
			Setting->SetDisplayName(LOCTEXT("FrameRateLimit_Mobile_Name", "Frame Rate Limit"));
			Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_Mobile_Description",
			                                        "Select a desired framerate. Use this to fine tune performance on your device."));

			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::MobileStyle));

			GraphicsQuality->AddSetting(Setting);
		}

		//----------------------------------------------------------------------------------
		{
			UGameSettingAction* Setting = NewObject<UGameSettingAction>();
			Setting->SetDevName(TEXT("AutoSetQuality"));
			Setting->SetDisplayName(LOCTEXT("AutoSetQuality_Name", "Auto-Set Quality"));
			Setting->SetDescriptionRichText(LOCTEXT("AutoSetQuality_Description",
			                                        "Automatically configure the graphics quality options based on a benchmark of the hardware."));

			Setting->SetDoesActionDirtySettings(true);
			Setting->SetActionText(LOCTEXT("AutoSetQuality_Action", "Auto-Set"));
			Setting->SetCustomAction([](ULocalPlayer* LocalPlayer)
			{
				const ULyraPlatformSpecificRenderingSettings* PlatformSettings =
					ULyraPlatformSpecificRenderingSettings::Get();
				if (PlatformSettings->FramePacingMode == ELyraFramePacingMode::MobileStyle)
				{
					ULyraSettingsLocal::Get()->ResetToMobileDeviceDefaults();
				}
				else
				{
					if (LocalPlayer && LocalPlayer->Implements<UPlayerSharedSettingsInterface>())
					{
						if (const auto ISharedSettings = CastChecked<IPlayerSharedSettingsInterface>(LocalPlayer))
						{
							// We don't save state until users apply the settings.
							constexpr bool bImmediatelySaveState = false;
							ISharedSettings->GetLocalSettings()->RunAutoBenchmark(bImmediatelySaveState);
						}
					}


					// const ULyraLocalPlayer* LyraLocalPlayer = CastChecked<ULyraLocalPlayer>(LocalPlayer);
					// // We don't save state until users apply the settings.
					// constexpr bool bImmediatelySaveState = false;
					// LyraLocalPlayer->GetLocalSettings()->RunAutoBenchmark(bImmediatelySaveState);
				}
			});

			Setting->AddEditCondition(MakeShared<FWhenCondition>(
				[](const ULocalPlayer* LocalPlayer, FGameSettingEditableState& InOutEditState)
				{
					const ULyraPlatformSpecificRenderingSettings* PlatformSettings =
						ULyraPlatformSpecificRenderingSettings::Get();
					const bool bCanUseDueToMobile = (PlatformSettings->FramePacingMode ==
						ELyraFramePacingMode::MobileStyle);

					if (LocalPlayer && LocalPlayer->Implements<UPlayerSharedSettingsInterface>())
					{
						if (const auto ISharedSettings = CastChecked<IPlayerSharedSettingsInterface>(LocalPlayer))
						{
							const bool bCanBenchmark = ISharedSettings->GetLocalSettings()->CanRunAutoBenchmark();

							if (!bCanUseDueToMobile && !bCanBenchmark)
							{
								InOutEditState.Kill(TEXT("Auto quality not supported"));
							}
						}
					}


					// const ULyraLocalPlayer* LyraLocalPlayer = CastChecked<ULyraLocalPlayer>(LocalPlayer);
					// const bool bCanBenchmark = LyraLocalPlayer->GetLocalSettings()->CanRunAutoBenchmark();
					//
					// if (!bCanUseDueToMobile && !bCanBenchmark)
					// {
					// 	InOutEditState.Kill(TEXT("Auto quality not supported"));
					// }
				}));

			if (MobileFPSType != nullptr)
			{
				MobileFPSType->AddEditDependency(Setting);
			}

			GraphicsQuality->AddSetting(Setting);

			AutoSetQuality = Setting;
		}
		//----------------------------------------------------------------------------------
		{
			ULyraSettingValueDiscrete_OverallQuality* Setting = NewObject<ULyraSettingValueDiscrete_OverallQuality>();
			Setting->SetDevName(TEXT("GraphicsQualityPresets"));
			Setting->SetDisplayName(LOCTEXT("GraphicsQualityPresets_Name", "Quality Presets"));
			Setting->SetDescriptionRichText(LOCTEXT("GraphicsQualityPresets_Description",
			                                        "Quality Preset allows you to adjust multiple video options at once. Try a few options to see what fits your preference and device's performance."));

			Setting->AddEditDependency(AutoSetQuality);

			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::ConsoleStyle,
				                                                      EFramePacingEditCondition::DisableIf));

			if (MobileFPSType != nullptr)
			{
				Setting->AddEditDependency(MobileFPSType);
				MobileFPSType->AddEditDependency(Setting);
			}

			GraphicsQuality->AddSetting(Setting);

			GraphicsQualityPresets = Setting;
		}
		//----------------------------------------------------------------------------------
		{
			UGameSettingValueScalarDynamic* Setting = NewObject<UGameSettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ResolutionScale"));
			Setting->SetDisplayName(LOCTEXT("ResolutionScale_Name", "3D Resolution"));
			Setting->SetDescriptionRichText(LOCTEXT("ResolutionScale_Description",
			                                        "3D resolution determines the resolution that objects are rendered in game, but does not affect the main menu.  Lower resolutions can significantly increase frame rate."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetResolutionScaleNormalized));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetResolutionScaleNormalized));
			Setting->SetDisplayFormat(UGameSettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support 3D Resolution")));
			//@TODO: Add support for 3d res on mobile

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);
			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("GlobalIlluminationQuality"),
				LOCTEXT("GlobalIlluminationQuality_Name", "Global Illumination"),
				LOCTEXT("GlobalIlluminationQuality_Description",
				        "Global Illumination controls the quality of dynamically calculated "
				        "indirect lighting bounces, sky shadowing and Ambient Occlusion. Settings of "
				        "'High' and above use more accurate ray tracing methods to solve lighting, but can reduce performance."),

				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetGlobalIlluminationQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetGlobalIlluminationQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameSettingEditCondition_VideoQuality>(
					TEXT("Platform does not support GlobalIlluminationQuality"))
			);

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				"Shadows",
				LOCTEXT("Shadows_Name", "Shadows"),
				LOCTEXT("Shadows_Description",
				        "Shadow quality determines the resolution and view distance of dynamic shadows. "
				        "Shadows improve visual quality and give better depth perception, but can reduce "
				        "performance."),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetShadowQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetShadowQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("ShadowLow", "Off"));
			Setting->AddOption(1, LOCTEXT("ShadowMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("ShadowHigh", "High"));
			Setting->AddOption(3, LOCTEXT("ShadowEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(
					TEXT("Platform does not support Shadows")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("AntiAliasing"),
				LOCTEXT("AntiAliasing_Name", "Anti-Aliasing"),
				LOCTEXT("AntiAliasing_Description",
				        "Anti-Aliasing reduces jaggy artifacts along geometry edges. "
				        "Increasing this setting will make edges look smoother, but can reduce performance."
				        " Higher settings mean more anti-aliasing."),

				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetAntiAliasingQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetAntiAliasingQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("AntiAliasingLow", "Off"));
			Setting->AddOption(1, LOCTEXT("AntiAliasingMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("AntiAliasingHigh", "High"));
			Setting->AddOption(3, LOCTEXT("AntiAliasingEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support Anti-Aliasing")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("ViewDistance"),
				LOCTEXT("ViewDistance_Name", "View Distance"),
				LOCTEXT("ViewDistance_Description",
				        "View distance determines how far away objects are culled for performance."),

				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetViewDistanceQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetViewDistanceQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("ViewDistanceNear", "Near"));
			Setting->AddOption(1, LOCTEXT("ViewDistanceMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("ViewDistanceFar", "Far"));
			Setting->AddOption(3, LOCTEXT("ViewDistanceEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support View Distance")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("TextureQuality"),
				LOCTEXT("TextureQuality_Name", "Textures"),

				LOCTEXT("TextureQuality_Description",
				        "Texture quality determines the resolution of textures in game. Increasing this setting will make objects more detailed, but can reduce performance."),

				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetTextureQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetTextureQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("TextureQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("TextureQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("TextureQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("TextureQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(TEXT("Platform does not support Texture quality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("VisualEffectQuality"),
				LOCTEXT("VisualEffectQuality_Name", "Effects"),
				LOCTEXT("VisualEffectQuality_Description",
				        "Effects determines the quality of visual effects and lighting in game. Increasing this setting will increase the quality of visual effects, but can reduce performance."),

				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetVisualEffectQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVisualEffectQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(
					TEXT("Platform does not support VisualEffectQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("ReflectionQuality"),
				LOCTEXT("ReflectionQuality_Name", "Reflections"),
				LOCTEXT("ReflectionQuality_Description",
				        "Reflection quality determines the resolution and accuracy of reflections.  Settings of 'High' and above use more accurate ray tracing methods to solve reflections, but can reduce performance."),

				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetReflectionQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetReflectionQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("VisualEffectQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(
					TEXT("Platform does not support ReflectionQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		{
			const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
				TEXT("PostProcessingQuality"),
				LOCTEXT("PostProcessingQuality_Name", "Post Processing"),
				LOCTEXT("PostProcessingQuality_Description",
				        "Post Processing effects include Motion Blur, Depth of Field and Bloom. Increasing this setting improves the quality of post process effects, but can reduce performance."),

				GET_LOCAL_SETTINGS_FUNCTION_PATH(GetPostProcessingQuality),
				GET_LOCAL_SETTINGS_FUNCTION_PATH(SetPostProcessingQuality), 3, 1);
			Setting->AddOption(0, LOCTEXT("PostProcessingQualityLow", "Low"));
			Setting->AddOption(1, LOCTEXT("PostProcessingQualityMedium", "Medium"));
			Setting->AddOption(2, LOCTEXT("PostProcessingQualityHigh", "High"));
			Setting->AddOption(3, LOCTEXT("PostProcessingQualityEpic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_VideoQuality>(
					TEXT("Platform does not support PostProcessingQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
	}

	// Advanced Graphics
	////////////////////////////////////////////////////////////////////////////////////
	{
		const auto AdvancedGraphics = UGameSettingCollection::CreateCollection(
			"AdvancedGraphics",LOCTEXT("AdvancedGraphics_Name", "Advanced Graphics"));
		Screen->AddSetting(AdvancedGraphics);

		//----------------------------------------------------------------------------------
		{
			UGameSettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameSettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("VerticalSync"));
			Setting->SetDisplayName(LOCTEXT("VerticalSync_Name", "Vertical Sync"));
			Setting->SetDescriptionRichText(LOCTEXT("VerticalSync_Description",
			                                        "Enabling Vertical Sync eliminates screen tearing by always rendering and presenting a full frame. Disabling Vertical Sync can give higher frame rate and better input response, but can result in horizontal screen tearing."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(IsVSyncEnabled));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVSyncEnabled));
			Setting->SetDefaultValue(false);

			Setting->AddEditCondition(
				MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::DesktopStyle));

			Setting->AddEditDependency(WindowModeSetting);
			Setting->AddEditCondition(MakeShared<FWhenCondition>(
				[WindowModeSetting](const ULocalPlayer*, FGameSettingEditableState& InOutEditState)
				{
					if (WindowModeSetting->GetValue<EWindowMode::Type>() != EWindowMode::Fullscreen)
					{
						InOutEditState.Disable(LOCTEXT("FullscreenNeededForVSync",
						                               "This feature only works if 'Window Mode' is set to 'Fullscreen'."));
					}
				}));

			AdvancedGraphics->AddSetting(Setting);
		}
	}

	return Screen;
}

void AddFrameRateOptions(const auto Setting)
{
	const FText FPSFormat = LOCTEXT("FPSFormat", "{0} FPS");
	for (const int32 Rate : GetDefault<ULyraPerformanceSettings>()->DesktopFrameRateLimits)
	{
		Setting->AddOption(static_cast<float>(Rate), FText::Format(FPSFormat, Rate));
	}
	Setting->AddOption(0.0f, LOCTEXT("UnlimitedFPS", "Unlimited"));
}

void ULyraGameSettingRegistry::InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen,
                                                                  ULocalPlayer* InLocalPlayer)
{
	//----------------------------------------------------------------------------------
	{
		const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
			TEXT("FrameRateLimit_OnBattery"),
			LOCTEXT("FrameRateLimit_OnBattery_Name", "Frame Rate Limit (On Battery)"),
			LOCTEXT("FrameRateLimit_OnBattery_Description",
			        "Frame rate limit when running on battery. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),

			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_OnBattery),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_OnBattery),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_OnBattery(), 1);

		Setting->AddEditCondition(
			MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::DesktopStyle));
		//@TODO: Hide if this device doesn't have a battery (no API for this right now)

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	{
		const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
			TEXT("FrameRateLimit_InMenu"),
			LOCTEXT("FrameRateLimit_InMenu_Name", "Frame Rate Limit (Menu)"),
			LOCTEXT("FrameRateLimit_InMenu_Description",
			        "Frame rate limit when in the menu. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),

			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_InMenu),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_InMenu),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_InMenu(), 1);
		Setting->AddEditCondition(
			MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	{
		const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
			TEXT("FrameRateLimit_WhenBackgrounded"),
			LOCTEXT("FrameRateLimit_WhenBackgrounded_Name", "Frame Rate Limit (Background)"),
			LOCTEXT("FrameRateLimit_WhenBackgrounded_Description",
			        "Frame rate limit when in the background. Set this lower for a more consistent frame rate or higher "
			        "for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),

			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_WhenBackgrounded),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_WhenBackgrounded),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_WhenBackgrounded(), 1);
		Setting->AddEditCondition(
			MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	{
		const auto Setting = UGameSettingValueDiscreteDynamic_Number::Create(
			TEXT("FrameRateLimit_Always"),
			LOCTEXT("FrameRateLimit_Always_Name", "Frame Rate Limit"),
			LOCTEXT("FrameRateLimit_Always_Description",
			        "Frame rate limit sets the highest frame rate that is allowed. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."),

			GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_Always),
			GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_Always),
			GetDefault<ULyraSettingsLocal>()->GetFrameRateLimit_Always(),
			1);

		Setting->AddEditCondition(
			MakeShared<FGameSettingEditCondition_FramePacingMode>(ELyraFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
}

#undef LOCTEXT_NAMESPACE
