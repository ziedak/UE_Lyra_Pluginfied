# ULyraSettingsShared

## Overview
`ULyraSettingsShared` is a class derived from `ULocalPlayerSaveGame`. It provides functionality for managing shared game settings that are stored in the cloud and are not machine-specific. These settings include colorblind options, gamepad configurations, subtitles, and more.

## Properties

### Color Blind Options
- **ColorBlindMode**: The selected colorblind mode.
  - **Type**: `EColorBlindMode`
  - **Default**: `EColorBlindMode::Off`
  - **Description**: Specifies the colorblind mode to apply.

- **ColorBlindStrength**: The strength of the colorblind effect.
  - **Type**: `int32`
  - **Default**: `10`

### Gamepad Settings
- **bForceFeedbackEnabled**: Whether force feedback is enabled for gamepads.
  - **Type**: `bool`
  - **Default**: `true`

- **GamepadMoveStickDeadZone**: Dead zone value for the gamepad's move stick.
  - **Type**: `float`

- **GamepadLookStickDeadZone**: Dead zone value for the gamepad's look stick.
  - **Type**: `float`

- **bTriggerHapticsEnabled**: Whether trigger haptics are enabled.
  - **Type**: `bool`
  - **Default**: `false`

- **TriggerHapticStrength**: The strength of trigger haptic effects.
  - **Type**: `uint8`
  - **Default**: `8`

### Subtitles
- **bEnableSubtitles**: Whether subtitles are enabled.
  - **Type**: `bool`
  - **Default**: `true`

- **SubtitleTextSize**: The size of subtitle text.
  - **Type**: `ESubtitleDisplayTextSize`
  - **Default**: `ESubtitleDisplayTextSize::Medium`

- **SubtitleTextColor**: The color of subtitle text.
  - **Type**: `ESubtitleDisplayTextColor`
  - **Default**: `ESubtitleDisplayTextColor::White`

### Background Audio
- **AllowAudioInBackground**: The background audio setting.
  - **Type**: `EBackgroundAudioSetting`
  - **Default**: `EBackgroundAudioSetting::Off`

### Culture/Language
- **PendingCulture**: The culture to apply.
  - **Type**: `FString`

- **bResetToDefaultCulture**: Whether to reset to the default culture.
  - **Type**: `bool`
  - **Default**: `false`

### Mouse Sensitivity
- **MouseSensitivityX**: Horizontal mouse sensitivity.
  - **Type**: `double`
  - **Default**: `1.0`

- **MouseSensitivityY**: Vertical mouse sensitivity.
  - **Type**: `double`
  - **Default**: `1.0`

- **TargetingMultiplier**: Multiplier applied while aiming down sights.
  - **Type**: `double`
  - **Default**: `0.5`

- **bInvertVerticalAxis**: Whether the vertical axis is inverted.
  - **Type**: `bool`
  - **Default**: `false`

- **bInvertHorizontalAxis**: Whether the horizontal axis is inverted.
  - **Type**: `bool`
  - **Default**: `false`

### Gamepad Sensitivity
- **GamepadLookSensitivityPreset**: Preset for gamepad look sensitivity.
  - **Type**: `EGamepadSensitivity`
  - **Default**: `EGamepadSensitivity::Normal`

- **GamepadTargetingSensitivityPreset**: Preset for gamepad targeting sensitivity.
  - **Type**: `EGamepadSensitivity`
  - **Default**: `EGamepadSensitivity::Normal`

## Methods

### Color Blind Options
- **GetColorBlindMode**: Retrieves the current colorblind mode.
  - **Return Type**: `EColorBlindMode`

- **SetColorBlindMode**: Sets the colorblind mode.
  - **Parameters**:
    - `EColorBlindMode InMode`: The colorblind mode to set.

- **GetColorBlindStrength**: Retrieves the colorblind strength.
  - **Return Type**: `int32`

- **SetColorBlindStrength**: Sets the colorblind strength.
  - **Parameters**:
    - `int32 InColorBlindStrength`: The strength to set.

### Gamepad Settings
- **GetForceFeedbackEnabled**: Checks if force feedback is enabled.
  - **Return Type**: `bool`

- **SetForceFeedbackEnabled**: Enables or disables force feedback.
  - **Parameters**:
    - `bool NewValue`: Whether to enable force feedback.

- **GetGamepadMoveStickDeadZone**: Retrieves the move stick dead zone value.
  - **Return Type**: `float`

- **SetGamepadMoveStickDeadZone**: Sets the move stick dead zone value.
  - **Parameters**:
    - `float NewValue`: The dead zone value to set.

### Subtitles
- **GetSubtitlesEnabled**: Checks if subtitles are enabled.
  - **Return Type**: `bool`

- **SetSubtitlesEnabled**: Enables or disables subtitles.
  - **Parameters**:
    - `bool Value`: Whether to enable subtitles.

- **ApplySubtitleOptions**: Applies the current subtitle settings.

### Background Audio
- **GetAllowAudioInBackgroundSetting**: Retrieves the background audio setting.
  - **Return Type**: `EBackgroundAudioSetting`

- **SetAllowAudioInBackgroundSetting**: Sets the background audio setting.
  - **Parameters**:
    - `EBackgroundAudioSetting NewValue`: The background audio setting to apply.

### Culture/Language
- **GetPendingCulture**: Retrieves the pending culture.
  - **Return Type**: `const FString&`

- **SetPendingCulture**: Sets the pending culture.
  - **Parameters**:
    - `const FString& NewCulture`: The culture to set.

- **ApplyCultureSettings**: Applies the current culture settings.

### Mouse Sensitivity
- **GetMouseSensitivityX**: Retrieves the horizontal mouse sensitivity.
  - **Return Type**: `double`

- **SetMouseSensitivityX**: Sets the horizontal mouse sensitivity.
  - **Parameters**:
    - `double NewValue`: The sensitivity value to set.

- **GetMouseSensitivityY**: Retrieves the vertical mouse sensitivity.
  - **Return Type**: `double`

- **SetMouseSensitivityY**: Sets the vertical mouse sensitivity.
  - **Parameters**:
    - `double NewValue`: The sensitivity value to set.

### Gamepad Sensitivity
- **GetGamepadLookSensitivityPreset**: Retrieves the gamepad look sensitivity preset.
  - **Return Type**: `EGamepadSensitivity`

- **SetLookSensitivityPreset**: Sets the gamepad look sensitivity preset.
  - **Parameters**:
    - `EGamepadSensitivity NewValue`: The sensitivity preset to set.

## Usage
1. Use `ULyraSettingsShared` to manage shared settings such as colorblind options, gamepad configurations, and subtitles.
2. Call the appropriate getter and setter methods to retrieve or modify settings.
3. Use `SaveSettings` to persist changes.

## Notes
- This class is designed to store settings that are shared across devices and users.
- It integrates with Unreal Engine's `USaveGame` system for cloud storage.

## See Also
- [ULocalPlayerSaveGame](https://docs.unrealengine.com/)
