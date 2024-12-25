// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "DataSource/GameSettingDataSourceDynamic.h" // IWYU pragma: keep
#include "GameSettingRegistry.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "Settings/LyraSettingsLocal.h" // IWYU pragma: keep

#include "LyraGameSettingRegistry.generated.h"


//--------------------------------------
// ULyraGameSettingRegistry
//--------------------------------------

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
/**
 * 
 */
UCLASS()
class ULyraGameSettingRegistry : public UGameSettingRegistry
{
	GENERATED_BODY()

public:
	static ULyraGameSettingRegistry* Get(ULocalPlayer* InLocalPlayer);

	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

	UGameSettingCollection* InitializeVideoSettings(ULocalPlayer* InLocalPlayer);
	void InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen, ULocalPlayer* InLocalPlayer);
	void AddPerformanceStatPage(UGameSettingCollection* Screen, ULocalPlayer* InLocalPlayer) const;

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
	UGameSettingCollectionPage* SetLanguageSettings(ULocalPlayer* InLocalPlayer);
	UGameSettingValueDiscreteDynamic_Bool* SetReplaySettings(ULocalPlayer* InLocalPlayer);
	UGameSettingValueDiscreteDynamic_Number* SetReplayLimitSettings(ULocalPlayer* InLocalPlayer);

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
