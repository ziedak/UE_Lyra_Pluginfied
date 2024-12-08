// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "DataSource/GameSettingDataSourceDynamic.h" // IWYU pragma: keep
#include "GameSettingRegistry.h"
#include "PlayerSharedSettings.h"
#include "Settings/LyraSettingsLocal.h" // IWYU pragma: keep
#include "Settings/LyraSettingsShared.h"

#include "LyraGameSettingRegistry.generated.h"

class ULocalPlayer;
class UObject;

//--------------------------------------
// ULyraGameSettingRegistry
//--------------------------------------

class UGameSettingCollection;
class ULocalPlayer;

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
GET_FUNCTION_NAME_STRING_CHECKED(IPlayerSharedSettings, GetSharedSettings),				\
GET_FUNCTION_NAME_STRING_CHECKED(ULyraSettingsShared, FunctionOrPropertyName)		\
}))

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName)							\
MakeShared<FGameSettingDataSourceDynamic>(TArray<FString>({								\
GET_FUNCTION_NAME_STRING_CHECKED(IPlayerSharedSettings, GetLocalSettings),				\
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
	ULyraGameSettingRegistry();

	static ULyraGameSettingRegistry* Get(ULocalPlayer* InLocalPlayer);

	virtual void SaveChanges() override;

protected:
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	virtual bool IsFinishedInitializing() const override;

	UGameSettingCollection* InitializeVideoSettings(ULocalPlayer* InLocalPlayer);
	void InitializeVideoSettings_FrameRates(UGameSettingCollection* Screen, ULocalPlayer* InLocalPlayer);
	void AddPerformanceStatPage(UGameSettingCollection* Screen, ULocalPlayer* InLocalPlayer) const;

	UGameSettingCollection* InitializeAudioSettings(ULocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGameplaySettings(ULocalPlayer* InLocalPlayer);

	UGameSettingCollection* InitializeMouseAndKeyboardSettings(ULocalPlayer* InLocalPlayer);
	UGameSettingCollection* InitializeGamepadSettings(ULocalPlayer* InLocalPlayer);

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
