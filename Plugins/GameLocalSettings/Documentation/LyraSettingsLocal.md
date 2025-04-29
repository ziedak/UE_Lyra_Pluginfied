# ULyraSettingsLocal

## Overview
`ULyraSettingsLocal` is a class derived from `UGameUserSettings`. It provides functionality for managing local game settings, including performance, display, audio, and input configurations.

## Properties

### Performance Stats
- **DisplayStatList**: A map of performance stats and their display modes.
  - **Type**: `TMap<EDisplayablePerformanceStat, EStatDisplayMode>`
  - **Description**: Stores the display mode for each performance stat.

- **PerfStatSettingsChangedEvent**: Event triggered when performance stat settings are changed.
  - **Type**: `FPerfStatSettingsChanged`
  - **Description**: Used to notify listeners of changes to performance stat settings.

### Display
- **DisplayGamma**: The gamma value for the display.
  - **Type**: `float`
  - **Default**: `2.2`
  - **Description**: Controls the brightness of the display.

- **FrameRateLimit_OnBattery**: Frame rate limit when running on battery.
  - **Type**: `float`

- **FrameRateLimit_InMenu**: Frame rate limit in menus.
  - **Type**: `float`

- **FrameRateLimit_WhenBackgrounded**: Frame rate limit when the application is in the background.
  - **Type**: `float`

- **SafeZoneScale**: The scale of the safe zone.
  - **Type**: `float`
  - **Default**: `-1`

### Audio
- **OverallVolume**: The overall volume level.
  - **Type**: `float`
  - **Default**: `1.0`

- **MusicVolume**: The music volume level.
  - **Type**: `float`
  - **Default**: `1.0`

- **SoundFXVolume**: The sound effects volume level.
  - **Type**: `float`
  - **Default**: `1.0`

- **DialogueVolume**: The dialogue volume level.
  - **Type**: `float`
  - **Default**: `1.0`

- **VoiceChatVolume**: The voice chat volume level.
  - **Type**: `float`
  - **Default**: `1.0`

- **AudioOutputDeviceId**: The ID of the selected audio output device.
  - **Type**: `FString`

### Replays
- **bShouldAutoRecordReplays**: Whether to automatically record replays.
  - **Type**: `bool`
  - **Default**: `false`

- **NumberOfReplaysToKeep**: The number of replays to keep.
  - **Type**: `int32`
  - **Default**: `5`

## Methods

### Performance Stats
- **GetPerfStatDisplayState**: Retrieves the display mode for a specific performance stat.
  - **Parameters**:
    - `EDisplayablePerformanceStat Stat`: The performance stat.
  - **Return Type**: `EStatDisplayMode`

- **SetPerfStatDisplayState**: Sets the display mode for a specific performance stat.
  - **Parameters**:
    - `EDisplayablePerformanceStat Stat`: The performance stat.
    - `EStatDisplayMode DisplayMode`: The display mode to set.

### Display
- **GetDisplayGamma**: Retrieves the display gamma value.
  - **Return Type**: `float`

- **SetDisplayGamma**: Sets the display gamma value.
  - **Parameters**:
    - `float InGamma`: The gamma value to set.

- **GetFrameRateLimit_OnBattery**: Retrieves the frame rate limit when on battery.
  - **Return Type**: `float`

- **SetFrameRateLimit_OnBattery**: Sets the frame rate limit when on battery.
  - **Parameters**:
    - `float NewLimitFPS`: The frame rate limit to set.

- **GetSafeZone**: Retrieves the safe zone scale.
  - **Return Type**: `float`

- **SetSafeZone**: Sets the safe zone scale.
  - **Parameters**:
    - `float Value`: The safe zone scale to set.

### Audio
- **GetOverallVolume**: Retrieves the overall volume level.
  - **Return Type**: `float`

- **SetOverallVolume**: Sets the overall volume level.
  - **Parameters**:
    - `float InVolume`: The volume level to set.

- **GetAudioOutputDeviceId**: Retrieves the ID of the selected audio output device.
  - **Return Type**: `FString`

- **SetAudioOutputDeviceId**: Sets the audio output device by ID.
  - **Parameters**:
    - `const FString& InAudioOutputDeviceId`: The ID of the audio output device to set.

### Replays
- **ShouldAutoRecordReplays**: Checks if replays should be automatically recorded.
  - **Return Type**: `bool`

- **SetShouldAutoRecordReplays**: Enables or disables automatic replay recording.
  - **Parameters**:
    - `bool bEnabled`: Whether to enable automatic replay recording.

- **GetNumberOfReplaysToKeep**: Retrieves the number of replays to keep.
  - **Return Type**: `int32`

- **SetNumberOfReplaysToKeep**: Sets the number of replays to keep.
  - **Parameters**:
    - `int32 InNumberOfReplays`: The number of replays to keep.

## Usage
1. Use `ULyraSettingsLocal` to manage local game settings such as performance, display, audio, and replays.
2. Call the appropriate getter and setter methods to retrieve or modify settings.
3. Use `LoadSettings` and `SaveSettings` to persist changes.

## Notes
- This class is designed to centralize the management of local game settings.
- It integrates with Unreal Engine's `UGameUserSettings` for seamless configuration.

## See Also
- [UGameUserSettings](https://docs.unrealengine.com/)
