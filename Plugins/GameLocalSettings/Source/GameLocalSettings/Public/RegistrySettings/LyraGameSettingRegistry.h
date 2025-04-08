// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "DataSource/GameSettingDataSourceDynamic.h" // IWYU pragma: keep
#include "GameSettingRegistry.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"

#include "CustomSettings/LyraSettingAction_SafeZoneEditor.h"
#include "CustomSettings/LyraSettingValueDiscrete_MobileFPSType.h"
#include "CustomSettings/LyraSettingValueDiscrete_OverallQuality.h"
#include "CustomSettings/LyraSettingValueDiscrete_PerfStat.h"
#include "CustomSettings/LyraSettingValueDiscrete_Resolution.h"

#include "Settings/LyraSettingsLocal.h" // IWYU pragma: keep

#include "LyraGameSettingRegistry.generated.h"


//--------------------------------------
// ULyraGameSettingRegistry
//--------------------------------------

class ULyraSettingValueDiscrete_Language;
class ULyraSettingValueDiscreteDynamic_AudioOutputDevice;
class UGameSettingCollectionPage;
class ULyraSettingsShared;
class UGameSettingCollection;
class ULocalPlayer;
class UObject;

DECLARE_LOG_CATEGORY_EXTERN(LogLyraGameSettingRegistry, Log, Log);

// #define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
// 	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
// 		GET_FUNCTION_NAME_STRING_CHECKED(ULocalPlayer, GetSharedSettings),				\
// 		GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsShared, FunctionOrPropertyName)		\
// 	}))
//
// #define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
// 	MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
// 		GET_FUNCTION_NAME_STRING_CHECKED(ULocalPlayer, GetLocalSettings),				\
// 		GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsLocal, FunctionOrPropertyName)		\
// 	}))
//@TODO not sure verify this is the right way to do this
#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
GET_FUNCTION_NAME_STRING_CHECKED(IPlayerSharedSettingsInterface, GetSharedSettings),				\
GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsShared, FunctionOrPropertyName)		\
}))

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
GET_FUNCTION_NAME_STRING_CHECKED(IPlayerSharedSettingsInterface, GetLocalSettings),				\
GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsLocal, FunctionOrPropertyName)		\
}))

// #define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
// MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
// GET_FUNCTION_NAME_STRING_CHECKED(UMyClass, GetSharedSettings),				\
// GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsShared, FunctionOrPropertyName)		\
// }))
//
// #define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
// MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
// GET_FUNCTION_NAME_STRING_CHECKED(UMyClass, GetLocalSettings),				\
// GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsLocal, FunctionOrPropertyName)		\
// }))
/**
 *
 */
UCLASS()
class GAMELOCALSETTINGS_API ULyraGameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()

public:
	static ULyraGameSettingRegistry* Get(ULocalPlayer* InLocalPlayer);

	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;
	bool IsOwningLocalPlayerValid() const;
	bool AreSharedSettingsAvailable() const;

	UGameSettingCollection* InitializeVideoSettings(ULocalPlayer* InLocalPlayer);

private:
	void AddDisplaySettings(UGameSettingCollection* Screen);
	UGameSettingValueDiscreteDynamic_Enum* CreateWindowModeSetting();
	ULyraSettingValueDiscrete_Resolution* CreateResolutionSetting(UGameSettingValueDiscreteDynamic_Enum* EditDependency);
	void AddGraphicsSettings(UGameSettingCollection* Screen);
	UGameSettingValueDiscreteDynamic_Enum* AddColorBlindModeSetting();
	UGameSettingValueDiscreteDynamic_Number* AddColorBlindStrength();
	UGameSettingValueScalarDynamic* AddBrightnessSetting();
	ULyraSettingAction_SafeZoneEditor* AddSafeZoneSetting();
	void AddGraphicsQualitySettings(UGameSettingCollection* Screen);
	UGameSettingValueDiscreteDynamic* CreateDeviceProfileSuffixSetting();
	ULyraSettingValueDiscrete_MobileFPSType* CreateMobileFPSTypeSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingAction* CreateAutoSetQualitySetting();
	ULyraSettingValueDiscrete_OverallQuality* CreateGraphicsQualityPresetsSetting();
	UGameSettingValueScalarDynamic* CreateResolutionScaleSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreateGlobalIlluminationQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreateShadowsSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreateAntiAliasingSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreateViewDistanceSetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreateTextureQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreateVisualEffectQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreateReflectionQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	UGameSettingValueDiscreteDynamic_Number* CreatePostProcessingQualitySetting(UGameSetting* AutoSetQuality, UGameSetting* GraphicsQualityPresets);
	void AddAdvancedGraphicsSettings(UGameSettingCollection* Screen);
	UGameSettingValueDiscreteDynamic_Bool* CreateVerticalSyncSettings(UGameSettingValueDiscreteDynamic_Enum* EditSetting);
	void InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen);
	void AddPerformanceStatPage(UGameSettingCollection* PerfStatsOuterCategory) const;
	UGameSettingCollectionPage* CreatePerformanceStatsPage() const;
	void AddPerformanceStats(UGameSettingCollectionPage* StatsPage) const;
	void AddNetworkStats(UGameSettingCollectionPage* StatsPage) const;
	ULyraSettingValueDiscrete_PerfStat* AddPerformanceStat(EDisplayablePerformanceStat Stat, const FText& DisplayName, const FText& Description) const;

protected:
	UGameSettingCollection* InitializeAudioSettings(ULocalPlayer* InLocalPlayer);

private:
	UGameSettingCollection* AddSoundSettings();
	UGameSettingCollectionPage* AddSubtitleSettings();
	UGameSettingCollection* AddVolumeSettings();
	ULyraSettingValueDiscreteDynamic_AudioOutputDevice* AddAudioOutputDeviceSetting();
	UGameSettingValueDiscreteDynamic_Enum* AddBackgroundAudioSetting();
	UGameSettingValueDiscreteDynamic_Bool* AddHeadphoneModeSetting();
	UGameSettingValueDiscreteDynamic_Bool* AddHDRAudioModeSetting();
	UGameSettingValueScalarDynamic* AddVolume(const FName& DevName,
	                                          const FText& DisplayName, const FText& Description,
	                                          const TSharedRef<FGameSettingDataSource>& Getter,
	                                          const TSharedRef<FGameSettingDataSource>& Setter,
	                                          const float DefaultValue);

protected:
	UGameSettingCollection* InitializeMouseAndKeyboardSettings(ULocalPlayer* InLocalPlayer);

private:
	UGameSettingValueScalarDynamic* AddMouseSensitivityYawSetting(const TRange<double>& Range, double Step);
	UGameSettingValueScalarDynamic* AddMouseSensitivityPitchSetting(const TRange<double>& Range, const double Step);
	UGameSettingValueScalarDynamic* AddMouseTargetingMultiplierSetting(const TRange<double>& Range, const double Step);
	UGameSettingValueDiscreteDynamic_Bool* AddInvertVerticalAxisSetting();
	UGameSettingValueDiscreteDynamic_Bool* AddInvertHorizontalAxisSetting();
	void AddKeyBindingSettings(UGameSettingCollection* Screen, const ULocalPlayer* InLocalPlayer);
	UGameSettingCollection* AddMouseSensitivitySettings();

protected:
	UGameSettingCollection* InitializeGamepadSettings(ULocalPlayer* InLocalPlayer);

private:
	UGameSettingValueDiscreteDynamic* CreateControllerHardwareSetting();
	UGameSettingValueDiscreteDynamic_Bool* CreateGamepadVibrationSetting();
	UGameSettingValueDiscreteDynamic_Bool* CreateInvertVerticalAxisSetting();
	UGameSettingValueDiscreteDynamic_Bool* CreateInvertHorizontalAxisSetting();
	UGameSettingCollection* CreateHardwareCollection();
	UGameSettingCollection* CreateGamepadBindingCollection();
	UGameSettingValueDiscreteDynamic_Enum* CreateLookSensitivityPresetSetting();
	UGameSettingValueDiscreteDynamic_Enum* CreateLookSensitivityPresetAdsSetting();
	UGameSettingCollection* CreateBasicSensitivityCollection();
	UGameSettingCollection* CreateDeadZoneCollection();
	UGameSettingValueScalarDynamic* CreateMoveStickDeadZoneSetting(float GamepadSensitivityMinimumLimit,
	                                                               float GamepadSensitivityMaximumLimit);
	UGameSettingValueScalarDynamic* CreateLookStickDeadZoneSetting(float GamepadSensitivityMinimumLimit,
	                                                               float GamepadSensitivityMaximumLimit);

protected:
	UGameSettingCollection* InitializeGameplaySettings(ULocalPlayer* InLocalPlayer);

private:
	ULyraSettingValueDiscrete_Language* SetLanguageSettings();
	UGameSettingValueDiscreteDynamic_Bool* SetReplaySettings();
	UGameSettingValueDiscreteDynamic_Number* SetReplayLimitSettings();

protected:
	UPROPERTY()
	TObjectPtr<UGameSettingCollection> VideoSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> AudioSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GameplaySettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> MouseAndKeyboardSettings;

	UPROPERTY()
	TObjectPtr<UGameSettingCollection> GamepadSettings;
};