// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/SaveGame.h"
#include "SubtitleDisplayOptions.h"

#include "UObject/ObjectPtr.h"
#include "LyraSettingsShared.generated.h"

class UObject;
struct FFrame;

UENUM(BlueprintType)
enum class EColorBlindMode : uint8
{
	Off,
	// Deuteranope (green weak/blind)
	Deuteranope,
	// Protanope (red weak/blind)
	Protanope,
	// Tritanope(blue weak / bind)
	Tritanope
};

UENUM(BlueprintType)
enum class EBackgroundAudioSetting : uint8
{
	Off,
	AllSounds,

	Num UMETA(Hidden),
};

UENUM(BlueprintType)
enum class EGamepadSensitivity : uint8
{
	Invalid = 0 UMETA(Hidden),

	Slow UMETA(DisplayName = "01 - Slow"),
	SlowPlus UMETA(DisplayName = "02 - Slow+"),
	SlowPlusPlus UMETA(DisplayName = "03 - Slow++"),
	Normal UMETA(DisplayName = "04 - Normal"),
	NormalPlus UMETA(DisplayName = "05 - Normal+"),
	NormalPlusPlus UMETA(DisplayName = "06 - Normal++"),
	Fast UMETA(DisplayName = "07 - Fast"),
	FastPlus UMETA(DisplayName = "08 - Fast+"),
	FastPlusPlus UMETA(DisplayName = "09 - Fast++"),
	Insane UMETA(DisplayName = "10 - Insane"),

	MAX UMETA(Hidden),
};

class ULocalPlayer;

/**
 * ULyraSettingsShared - The "Shared" settings are stored as part of the USaveGame system, these settings are not machine
 * specific like the local settings, and are safe to store in the cloud - and 'share' them.  Using the save game system
 * we can also store settings per player, so things like controller keybind preferences should go here, because if those
 * are stored in the local settings all users would get them.
 *
 */
UCLASS()
class GAMELOCALSETTINGS_API ULyraSettingsShared : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:
	DECLARE_EVENT_OneParam(ULyraSettingsShared, FOnSettingChangedEvent, const ULyraSettingsShared* Settings);

	FOnSettingChangedEvent OnSettingChanged;

	ULyraSettingsShared();

	//~ULocalPlayerSaveGame interface
	// 0 = before subclassing ULocalPlayerSaveGame
	// 1 = first proper version
	virtual int32 GetLatestDataVersion() const override { return 1; };
	//~End of ULocalPlayerSaveGame interface

	/** Creates a temporary settings object, this will be replaced by one loaded from the user's save_game */
	static ULyraSettingsShared* CreateTemporarySettings(const ULocalPlayer* LocalPlayer);

	/** Synchronously loads a settings object, this is not valid to call before login */
	static ULyraSettingsShared* LoadOrCreateSettings(const ULocalPlayer* LocalPlayer);

	DECLARE_DELEGATE_OneParam(FOnSettingsLoadedEvent, ULyraSettingsShared* Settings);

	/** Starts an async load of the settings object, calls Delegate on completion */
	static bool AsyncLoadOrCreateSettings(const ULocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate);

	/** Saves the settings to disk */
	void SaveSettings();
	void ApplyInputSettings() const;

	/** Applies the current settings to the player */
	void ApplySettings();

#pragma region ColorBlindOptions

	////////////////////////////////////////////////////////
	// Color Blind Options
	UFUNCTION()
	EColorBlindMode GetColorBlindMode() const { return ColorBlindMode; };
	UFUNCTION()
	void SetColorBlindMode(EColorBlindMode InMode);

	UFUNCTION()
	int32 GetColorBlindStrength() const { return ColorBlindStrength; };
	UFUNCTION()
	void SetColorBlindStrength(int32 InColorBlindStrength);

private:
	UPROPERTY()
	EColorBlindMode ColorBlindMode = EColorBlindMode::Off;

	UPROPERTY()
	int32 ColorBlindStrength = 10;
#pragma endregion
#pragma region GamepadVibration
	////////////////////////////////////////////////////////
	// Gamepad Vibration

public:
	UFUNCTION()
	bool GetForceFeedbackEnabled() const { return bForceFeedbackEnabled; }

	UFUNCTION()
	void SetForceFeedbackEnabled(const bool NewValue) { ChangeValueAndDirty(bForceFeedbackEnabled, NewValue); }

private:
	/** Is force feedback enabled when a controller is being used? */
	UPROPERTY()
	bool bForceFeedbackEnabled = true;
#pragma endregion
#pragma region GamepadDeadzone
	////////////////////////////////////////////////////////
	// Gamepad Deadzone

public:
	/** Getter for gamepad move stick dead zone value. */
	UFUNCTION()
	float GetGamepadMoveStickDeadZone() const { return GamepadMoveStickDeadZone; }

	/** Setter for gamepad move stick dead zone value. */
	UFUNCTION()
	void SetGamepadMoveStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadMoveStickDeadZone, NewValue); }

	/** Getter for gamepad look stick dead zone value. */
	UFUNCTION()
	float GetGamepadLookStickDeadZone() const { return GamepadLookStickDeadZone; }

	/** Setter for gamepad look stick dead zone value. */
	UFUNCTION()
	void SetGamepadLookStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadLookStickDeadZone, NewValue); }

private:
	/** Holds the gamepad move stick dead zone value. */
	UPROPERTY()
	float GamepadMoveStickDeadZone;

	/** Holds the gamepad look stick dead zone value. */
	UPROPERTY()
	float GamepadLookStickDeadZone;
#pragma endregion
#pragma region GamepadTriggerHaptics
	////////////////////////////////////////////////////////
	// Gamepad Trigger Haptics
public:
	UFUNCTION()
	bool GetTriggerHapticsEnabled() const { return bTriggerHapticsEnabled; }

	UFUNCTION()
	void SetTriggerHapticsEnabled(const bool NewValue) { ChangeValueAndDirty(bTriggerHapticsEnabled, NewValue); }

	UFUNCTION()
	bool GetTriggerPullUsesHapticThreshold() const { return bTriggerPullUsesHapticThreshold; }

	UFUNCTION()
	void SetTriggerPullUsesHapticThreshold(const bool NewValue)
	{
		ChangeValueAndDirty(bTriggerPullUsesHapticThreshold, NewValue);
	}

	UFUNCTION()
	uint8 GetTriggerHapticStrength() const { return TriggerHapticStrength; }

	UFUNCTION()
	void SetTriggerHapticStrength(const uint8 NewValue) { ChangeValueAndDirty(TriggerHapticStrength, NewValue); }

	UFUNCTION()
	uint8 GetTriggerHapticStartPosition() const { return TriggerHapticStartPosition; }

	UFUNCTION()
	void SetTriggerHapticStartPosition(const uint8 NewValue)
	{
		ChangeValueAndDirty(TriggerHapticStartPosition, NewValue);
	}

private:
	/** Are trigger haptics enabled? */
	UPROPERTY()
	bool bTriggerHapticsEnabled = false;
	/** Does the game use the haptic feedback as its threshold for judging button presses? */
	UPROPERTY()
	bool bTriggerPullUsesHapticThreshold = true;
	/** The strength of the trigger haptic effects. */
	UPROPERTY()
	uint8 TriggerHapticStrength = 8;
	/** The start position of the trigger haptic effects */
	UPROPERTY()
	uint8 TriggerHapticStartPosition = 0;
#pragma endregion
#pragma region Subtitles
	////////////////////////////////////////////////////////
	// Subtitles
public:
	UFUNCTION()
	bool GetSubtitlesEnabled() const { return bEnableSubtitles; }

	UFUNCTION()
	void SetSubtitlesEnabled(bool Value) { ChangeValueAndDirty(bEnableSubtitles, Value); }

	UFUNCTION()
	ESubtitleDisplayTextSize GetSubtitlesTextSize() const { return SubtitleTextSize; }

	UFUNCTION()
	void SetSubtitlesTextSize(const ESubtitleDisplayTextSize Value)
	{
		ChangeValueAndDirty(SubtitleTextSize, Value);
		ApplySubtitleOptions();
	}

	UFUNCTION()
	ESubtitleDisplayTextColor GetSubtitlesTextColor() const { return SubtitleTextColor; }

	UFUNCTION()
	void SetSubtitlesTextColor(const ESubtitleDisplayTextColor Value)
	{
		ChangeValueAndDirty(SubtitleTextColor, Value);
		ApplySubtitleOptions();
	}

	UFUNCTION()
	ESubtitleDisplayTextBorder GetSubtitlesTextBorder() const { return SubtitleTextBorder; }

	UFUNCTION()
	void SetSubtitlesTextBorder(const ESubtitleDisplayTextBorder Value)
	{
		ChangeValueAndDirty(SubtitleTextBorder, Value);
		ApplySubtitleOptions();
	}

	UFUNCTION()
	ESubtitleDisplayBackgroundOpacity GetSubtitlesBackgroundOpacity() const { return SubtitleBackgroundOpacity; }

	UFUNCTION()
	void SetSubtitlesBackgroundOpacity(const ESubtitleDisplayBackgroundOpacity Value)
	{
		ChangeValueAndDirty(SubtitleBackgroundOpacity, Value);
		ApplySubtitleOptions();
	}

	void ApplySubtitleOptions() const;

private:
	UPROPERTY()
	bool bEnableSubtitles = true;

	UPROPERTY()
	ESubtitleDisplayTextSize SubtitleTextSize = ESubtitleDisplayTextSize::Medium;

	UPROPERTY()
	ESubtitleDisplayTextColor SubtitleTextColor = ESubtitleDisplayTextColor::White;

	UPROPERTY()
	ESubtitleDisplayTextBorder SubtitleTextBorder = ESubtitleDisplayTextBorder::None;

	UPROPERTY()
	ESubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity = ESubtitleDisplayBackgroundOpacity::Medium;
#pragma endregion
#pragma region SharedAudioSettings
	////////////////////////////////////////////////////////
	// Shared audio settings
public:
	UFUNCTION()
	EBackgroundAudioSetting GetAllowAudioInBackgroundSetting() const { return AllowAudioInBackground; }

	UFUNCTION()
	void SetAllowAudioInBackgroundSetting(EBackgroundAudioSetting NewValue);

	void ApplyBackgroundAudioSettings() const;

private:
	UPROPERTY()
	EBackgroundAudioSetting AllowAudioInBackground = EBackgroundAudioSetting::Off;
#pragma endregion
#pragma region CultureLanguage
	////////////////////////////////////////////////////////
	// Culture / language
public:
	/** Gets the pending culture */
	const FString& GetPendingCulture() const { return PendingCulture; };

	/** Sets the pending culture to apply */
	void SetPendingCulture(const FString& NewCulture);

	// Called when the culture changes.
	void OnCultureChanged();

	/** Clears the pending culture to apply */
	void ClearPendingCulture() { PendingCulture.Reset(); };

	bool IsUsingDefaultCulture() const;

	void ResetToDefaultCulture();
	bool ShouldResetToDefaultCulture() const { return bResetToDefaultCulture; }

	void ApplyCultureSettings();
	void ApplyDefaultCulture();
	void ApplyPendingCulture() const;
	void ResetCultureToCurrentSettings();

private:
	/** The pending culture to apply */
	UPROPERTY(Transient)
	FString PendingCulture;

	/* If true, resets the culture to default. */
	bool bResetToDefaultCulture = false;
#pragma endregion
#pragma region MouseSensitivity
	////////////////////////////////////////////////////////
	// Mouse Sensitivity
public:
	UFUNCTION()
	double GetMouseSensitivityX() const { return MouseSensitivityX; }

	UFUNCTION()
	void SetMouseSensitivityX(const double NewValue)
	{
		ChangeValueAndDirty(MouseSensitivityX, NewValue);
		ApplyInputSensitivity();
	}

	UFUNCTION()
	double GetMouseSensitivityY() const { return MouseSensitivityY; }

	UFUNCTION()
	void SetMouseSensitivityY(const double NewValue)
	{
		ChangeValueAndDirty(MouseSensitivityY, NewValue);
		ApplyInputSensitivity();
	}

	UFUNCTION()
	double GetTargetingMultiplier() const { return TargetingMultiplier; }

	UFUNCTION()
	void SetTargetingMultiplier(const double NewValue)
	{
		ChangeValueAndDirty(TargetingMultiplier, NewValue);
		ApplyInputSensitivity();
	}

	UFUNCTION()
	bool GetInvertVerticalAxis() const { return bInvertVerticalAxis; }

	UFUNCTION()
	void SetInvertVerticalAxis(const bool NewValue)
	{
		ChangeValueAndDirty(bInvertVerticalAxis, NewValue);
		ApplyInputSensitivity();
	}

	UFUNCTION()
	bool GetInvertHorizontalAxis() const { return bInvertHorizontalAxis; }

	UFUNCTION()
	void SetInvertHorizontalAxis(const bool NewValue)
	{
		ChangeValueAndDirty(bInvertHorizontalAxis, NewValue);
		ApplyInputSensitivity();
	}

private:
	/** Holds the mouse horizontal sensitivity */
	UPROPERTY()
	double MouseSensitivityX = 1.0;

	/** Holds the mouse vertical sensitivity */
	UPROPERTY()
	double MouseSensitivityY = 1.0;

	/** Multiplier applied while Aiming down sights. */
	UPROPERTY()
	double TargetingMultiplier = 0.5;

	/** If true then the vertical look axis should be inverted */
	UPROPERTY()
	bool bInvertVerticalAxis = false;

	/** If true then the horizontal look axis should be inverted */
	UPROPERTY()
	bool bInvertHorizontalAxis = false;
#pragma endregion
#pragma region GamepadSensitivity
	////////////////////////////////////////////////////////
	// Gamepad Sensitivity
public:
	UFUNCTION()
	EGamepadSensitivity GetGamepadLookSensitivityPreset() const { return GamepadLookSensitivityPreset; }

	UFUNCTION()
	void SetLookSensitivityPreset(const EGamepadSensitivity NewValue)
	{
		ChangeValueAndDirty(GamepadLookSensitivityPreset, NewValue);
		ApplyInputSensitivity();
	}

	UFUNCTION()
	EGamepadSensitivity GetGamepadTargetingSensitivityPreset() const { return GamepadTargetingSensitivityPreset; }

	UFUNCTION()
	void SetGamepadTargetingSensitivityPreset(EGamepadSensitivity NewValue)
	{
		ChangeValueAndDirty(GamepadTargetingSensitivityPreset, NewValue);
		ApplyInputSensitivity();
	}

	void ApplyInputSensitivity() const;

private:
	UPROPERTY()
	EGamepadSensitivity GamepadLookSensitivityPreset = EGamepadSensitivity::Normal;
	UPROPERTY()
	EGamepadSensitivity GamepadTargetingSensitivityPreset = EGamepadSensitivity::Normal;

#pragma endregion
#pragma region DirtyChangeReporting
	/// Dirty and Change Reporting
public:
	bool IsDirty() const { return bIsDirty; }
	void ClearDirtyFlag() { bIsDirty = false; }

private:
	template <typename T>
	bool ChangeValueAndDirty(T& CurrentValue, const T& NewValue)
	{
		if (CurrentValue == NewValue) { return false; }

		CurrentValue = NewValue;
		bIsDirty = true;
		OnSettingChanged.Broadcast(this);
		return true;
	}

	bool bIsDirty = false;
#pragma endregion
};
