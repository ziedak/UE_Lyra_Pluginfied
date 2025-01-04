// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameUserSettings.h"
// #include "InputConfig/LyraMappableConfigPair.h"
#include "InputCoreTypes.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "Performance/LyraPerformanceSettings.h"

#include "LyraSettingsLocal.generated.h"

enum class ECommonInputType : uint8;
enum class ELyraDisplayablePerformanceStat : uint8;
enum class ELyraStatDisplayMode : uint8;

class ULocalPlayer;
class UObject;
class UPlayerMappableInputConfig;
class USoundControlBus;
class USoundControlBusMix;
struct FFrame;

USTRUCT()
struct FLyraScalabilitySnapshot
{
	GENERATED_BODY()

	FLyraScalabilitySnapshot();

	Scalability::FQualityLevels Qualities;
	bool bActive = false;
	bool bHasOverrides = false;
};

/**
 * ULyraSettingsLocal
 */
UCLASS()
class GAMELOCALSETTINGS_API ULyraSettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

public:
	ULyraSettingsLocal();

	static ULyraSettingsLocal* Get();

	//~UObject interface
	virtual void BeginDestroy() override;
	//~End of UObject interface

	//~UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ConfirmVideoMode() override;
	virtual float GetEffectiveFrameRateLimit() override;
	virtual void ResetToCurrentSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	virtual int32 GetOverallScalabilityLevel() const override;
	virtual void SetOverallScalabilityLevel(int32 Value) override;

	void OnExperienceLoaded();
	void OnHotfixDeviceProfileApplied();

	//////////////////////////////////////////////////////////////////
	// Frontend state

	void SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd);

protected:
	bool ShouldUseFrontendPerformanceSettings() const;

private:
	bool bInFrontEndForPerformancePurposes = false;

	//////////////////////////////////////////////////////////////////
	// Performance stats
public:
	/** Returns the display mode for the specified performance stat */
	ELyraStatDisplayMode GetPerfStatDisplayState(ELyraDisplayablePerformanceStat Stat) const;

	/** Sets the display mode for the specified performance stat */
	void SetPerfStatDisplayState(ELyraDisplayablePerformanceStat Stat, ELyraStatDisplayMode DisplayMode);

	/** Fired when the display state for a performance stat has changed, or the settings are applied */
	DECLARE_EVENT(ULyraSettingsLocal, FPerfStatSettingsChanged);

	FPerfStatSettingsChanged& OnPerfStatDisplayStateChanged() { return PerfStatSettingsChangedEvent; }

private:
	// List of stats to display in the HUD
	UPROPERTY(Config)
	TMap<ELyraDisplayablePerformanceStat, ELyraStatDisplayMode> DisplayStatList;

	// Event for display stat widget containers to bind to
	FPerfStatSettingsChanged PerfStatSettingsChangedEvent;

	//////////////////////////////////////////////////////////////////
	// Brightness/Gamma
public:
	UFUNCTION()
	float GetDisplayGamma() const { return DisplayGamma; };
	UFUNCTION()
	void SetDisplayGamma(float InGamma);

private:
	void ApplyDisplayGamma() const;

	UPROPERTY(Config)
	float DisplayGamma = 2.2;

	//////////////////////////////////////////////////////////////////
	// Display
public:
	UFUNCTION()
	float GetFrameRateLimit_OnBattery() const { return FrameRateLimit_OnBattery; };
	UFUNCTION()
	void SetFrameRateLimit_OnBattery(float NewLimitFPS);

	UFUNCTION()
	float GetFrameRateLimit_InMenu() const { return FrameRateLimit_InMenu; };
	UFUNCTION()
	void SetFrameRateLimit_InMenu(float NewLimitFPS);

	UFUNCTION()
	float GetFrameRateLimit_WhenBackgrounded() const { return FrameRateLimit_WhenBackgrounded; };
	UFUNCTION()
	void SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS);

	UFUNCTION()
	float GetFrameRateLimit_Always() const;
	UFUNCTION()
	void SetFrameRateLimit_Always(float NewLimitFPS);

protected:
	void UpdateEffectiveFrameRateLimit();

private:
	UPROPERTY(Config)
	float FrameRateLimit_OnBattery;
	UPROPERTY(Config)
	float FrameRateLimit_InMenu;
	UPROPERTY(Config)
	float FrameRateLimit_WhenBackgrounded;

	//////////////////////////////////////////////////////////////////
	// Display - Mobile quality settings
public:
	static int32 GetDefaultMobileFrameRate();
	static int32 GetMaxMobileFrameRate();

	static bool IsSupportedMobileFramePace(int32 TestFPS);

	// Returns the first frame rate at which overall quality is restricted/limited by the current device profile
	int32 GetFirstFrameRateWithQualityLimit() const;

	// Returns the lowest quality at which there's a limit on the overall frame rate (or -1 if there is no limit)
	int32 GetLowestQualityWithFrameRateLimit() const;

	void ResetToMobileDeviceDefaults();

	int32 GetMaxSupportedOverallQualityLevel() const;

private:
	void SetMobileFPSMode(int32 NewLimitFPS);

	void ClampMobileResolutionQuality(int32 TargetFPS);
	void RemapMobileResolutionQuality(int32 FromFPS, int32 ToFPS);

	void ClampMobileFPSQualityLevels(bool bWriteBack);
	void ClampMobileQuality();

	int32 GetHighestLevelOfAnyScalabilityChannel() const;

	/* Modifies the input levels based on the active mode's overrides */
	void OverrideQualityLevelsToScalabilityMode(const FLyraScalabilitySnapshot& InMode,
	                                            Scalability::FQualityLevels& InOutLevels) const;

	/* Clamps the input levels based on the active device profile's default allowed levels */
	void ClampQualityLevelsToDeviceProfile(const Scalability::FQualityLevels& ClampLevels,
	                                       Scalability::FQualityLevels& InOutLevels) const;

public:
	int32 GetDesiredMobileFrameRateLimit() const { return DesiredMobileFrameRateLimit; }

	void SetDesiredMobileFrameRateLimit(int32 NewLimitFPS);

private:
	UPROPERTY(Config)
	int32 MobileFrameRateLimit = 30;

	FLyraScalabilitySnapshot DeviceDefaultScalabilitySettings;

	bool bSettingOverallQualityGuard = false;

	int32 DesiredMobileFrameRateLimit = 0;

	//////////////////////////////////////////////////////////////////
	// Display - Console quality presets
public:
	UFUNCTION()
	FString GetDesiredDeviceProfileQualitySuffix() const { return DesiredUserChosenDeviceProfileSuffix; };
	UFUNCTION()
	void SetDesiredDeviceProfileQualitySuffix(const FString& InDesiredSuffix) { DesiredUserChosenDeviceProfileSuffix = InDesiredSuffix; };

protected:
	/** Updates device profiles, FPS mode etc for the current game mode */
	void UpdateGameModeDeviceProfileAndFps();
	FString GetBasePlatformName() const;
	FName GetPlatformName() const;
	TArray<FString> BuildComposedNamesToFind(const FString& BasePlatformName,
	                                         const FString& EffectiveUserSuffix,
	                                         const FString& ExperienceSuffix) const;
	FString FindActualProfileToApply(UDeviceProfileManager& Manager,
	                                 const TArray<FString>& ComposedNamesToFind,
	                                 const FName& PlatformName) const;
	FString GetEffectiveUserSuffix(const TArray<FLyraQualityDeviceProfileVariant>& UserFacingVariants,
	                               const int32 PlatformMaxRefreshRate) const;
	void ApplyFrameSyncType() const;
	void ApplyTargetFPS() const;
	void ApplyDeviceProfileIfNeeded(UDeviceProfileManager& Manager, const FString& ActualProfileToApply);
	void RestoreDefaultDeviceProfile(UDeviceProfileManager& Manager);
	void ApplyNewDeviceProfile(UDeviceProfileManager& Manager, const FString& ActualProfileToApply);
	void UpdateFramePacing(ELyraFramePacingMode FramePacingMode);
	void UpdateConsoleFramePacing() const;
	void UpdateDesktopFramePacing();
	void UpdateMobileFramePacing();

	void UpdateDynamicResFrameTime(float TargetFPS) const;

private:
	UPROPERTY(Transient)
	FString DesiredUserChosenDeviceProfileSuffix;

	UPROPERTY(Transient)
	FString CurrentAppliedDeviceProfileOverrideSuffix;

	UPROPERTY(config)
	FString UserChosenDeviceProfileSuffix;

	UPROPERTY(config)
	bool bMobileDisableResolutionReset = true;
	//////////////////////////////////////////////////////////////////
	// Audio - Volume
public:
	DECLARE_EVENT_OneParam(ULyraSettingsLocal, FAudioDeviceChanged, const FString& /*DeviceId*/);

	FAudioDeviceChanged OnAudioOutputDeviceChanged;

	/** Returns if we're using headphone mode (HRTF) **/
	UFUNCTION()
	bool IsHeadphoneModeEnabled() const { return bUseHeadphoneMode; };

	/** Enables or disables headphone mode (HRTF) - NOTE this setting will be overruled if au.DisableBinauralSpatialization is set */
	UFUNCTION()
	void SetHeadphoneModeEnabled(bool bEnabled);

	/** Returns if we can enable/disable headphone mode (i.e., if it's not forced on or off by the platform) */
	UFUNCTION()
	bool CanModifyHeadphoneModeEnabled() const;

	/** Whether we *want* to use headphone mode (HRTF); may or may not actually be applied **/
	UPROPERTY(Transient)
	bool bDesiredHeadphoneMode;

private:
	/** Whether to use headphone mode (HRTF) **/
	UPROPERTY(config)
	bool bUseHeadphoneMode;

public:
	/** Returns if we're using High Dynamic Range Audio mode (HDR Audio) **/
	UFUNCTION()
	bool IsHDRAudioModeEnabled() const { return bUseHDRAudioMode; };

	/** Enables or disables High Dynamic Range Audio mode (HDR Audio) */
	UFUNCTION()
	void SetHDRAudioModeEnabled(bool bEnabled);

	/** Whether to use High Dynamic Range Audio mode (HDR Audio) **/
	UPROPERTY(config)
	bool bUseHDRAudioMode;

	/** Returns true if this platform can run the auto benchmark */
	UFUNCTION(BlueprintCallable, Category = Settings)
	bool CanRunAutoBenchmark() const;

	/** Returns true if this user should run the auto benchmark as it has never been run */
	UFUNCTION(BlueprintCallable, Category = Settings)
	bool ShouldRunAutoBenchmarkAtStartup() const;

	/** Run the auto benchmark, optionally saving right away */
	UFUNCTION(BlueprintCallable, Category = Settings)
	void RunAutoBenchmark(bool bSaveImmediately);

	/** Apply just the quality scalability settings */
	void ApplyScalabilitySettings() const;

	UFUNCTION()
	float GetOverallVolume() const { return OverallVolume; };
	UFUNCTION()
	void SetOverallVolume(float InVolume);

	UFUNCTION()
	float GetMusicVolume() const { return MusicVolume; };
	UFUNCTION()
	void SetMusicVolume(float InVolume);

	UFUNCTION()
	float GetSoundFXVolume() const { return SoundFXVolume; };
	UFUNCTION()
	void SetSoundFXVolume(float InVolume);

	UFUNCTION()
	float GetDialogueVolume() const { return DialogueVolume; };
	UFUNCTION()
	void SetDialogueVolume(float InVolume);

	UFUNCTION()
	float GetVoiceChatVolume() const { return VoiceChatVolume; };
	UFUNCTION()
	void SetVoiceChatVolume(float InVolume);

	//////////////////////////////////////////////////////////////////
	// Audio - Sound
	/** Returns the user's audio device id */
	UFUNCTION()
	FString GetAudioOutputDeviceId() const { return AudioOutputDeviceId; }

	/** Sets the user's audio device by id */
	UFUNCTION()
	void SetAudioOutputDeviceId(const FString& InAudioOutputDeviceId);

private:
	UPROPERTY(Config)
	FString AudioOutputDeviceId;

	void SetVolumeForSoundClass(FName ChannelName, float InVolume);


	//////////////////////////////////////////////////////////////////
	// Safezone
public:
	UFUNCTION()
	bool IsSafeZoneSet() const { return SafeZoneScale != -1; }

	UFUNCTION()
	float GetSafeZone() const { return SafeZoneScale >= 0 ? SafeZoneScale : 0; }

	UFUNCTION()
	void SetSafeZone(const float Value)
	{
		SafeZoneScale = Value;
		ApplySafeZoneScale();
	}

	void ApplySafeZoneScale() const;
	void SetVolume(FName VolumeType, float InVolume);

private:
	void SetVolumeForControlBus(USoundControlBus* InSoundControlBus, float InVolume);
	USoundControlBus* LoadControlBus(UObject* ObjPath, const FString& VolumeType);
	//////////////////////////////////////////////////////////////////
	// Keybindings
public:
	// Sets the controller representation to use, a single platform might support multiple kinds of controllers.  For
	// example, Win64 games could be played with both an XBox or Playstation controller.
	UFUNCTION()
	void SetControllerPlatform(const FName InControllerPlatform);

	UFUNCTION()
	FName GetControllerPlatform() const { return ControllerPlatform; };

private:
	void LoadUserControlBusMix();

	UPROPERTY(Config)
	float OverallVolume = 1.0f;
	UPROPERTY(Config)
	float MusicVolume = 1.0f;
	UPROPERTY(Config)
	float SoundFXVolume = 1.0f;
	UPROPERTY(Config)
	float DialogueVolume = 1.0f;
	UPROPERTY(Config)
	float VoiceChatVolume = 1.0f;

	UPROPERTY(Transient)
	TMap<FName/*SoundClassName*/, TObjectPtr<USoundControlBus>> ControlBusMap;

	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> ControlBusMix = nullptr;

	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;

	UPROPERTY(Config)
	float SafeZoneScale = -1;

	/**
	 * The name of the controller the player is using.  This is maps to the name of a UCommonInputBaseControllerData
	 * that is available on this current platform.  The gamepad data are registered per platform, you'll find them
	 * in <Platform>Game.ini files listed under +ControllerData=...
	 */
	UPROPERTY(Config)
	FName ControllerPlatform;

	UPROPERTY(Config)
	FName ControllerPreset = TEXT("Default");

	/** The name of the current input config that the user has selected. */
	UPROPERTY(Config)
	FName InputConfigName = TEXT("Default");

	// Replays
public:
	UFUNCTION()
	bool ShouldAutoRecordReplays() const { return bShouldAutoRecordReplays; }

	UFUNCTION()
	void SetShouldAutoRecordReplays(const bool bEnabled) { bShouldAutoRecordReplays = bEnabled; }

	UFUNCTION()
	int32 GetNumberOfReplaysToKeep() const { return NumberOfReplaysToKeep; }

	UFUNCTION()
	void SetNumberOfReplaysToKeep(const int32 InNumberOfReplays) { NumberOfReplaysToKeep = InNumberOfReplays; }

private:
	UPROPERTY(Config)
	bool bShouldAutoRecordReplays = false;

	UPROPERTY(Config)
	int32 NumberOfReplaysToKeep = 5;

	void OnAppActivationStateChanged(bool bIsActive);
	void ReapplyThingsDueToPossibleDeviceProfileChange();

	FDelegateHandle OnApplicationActivationStateChangedHandle;
};