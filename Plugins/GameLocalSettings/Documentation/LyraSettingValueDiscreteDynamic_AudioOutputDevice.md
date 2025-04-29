# ULyraSettingValueDiscreteDynamic_AudioOutputDevice

## Overview
`ULyraSettingValueDiscreteDynamic_AudioOutputDevice` is a class derived from `UGameSettingValueDiscreteDynamic`. It provides functionality for managing and selecting audio output devices dynamically.

## Properties
- **OutputDevices**: A list of available audio output devices.
  - **Type**: `TArray<FAudioOutputDeviceInfo>`
  - **Description**: Stores information about all detected audio output devices.

- **CurrentDeviceId**: The ID of the currently selected audio output device.
  - **Type**: `FString`
  - **Description**: Represents the device ID currently in use.

- **SystemDefaultDeviceId**: The ID of the system's default audio output device.
  - **Type**: `FString`
  - **Description**: Represents the default audio device as determined by the system.

- **LastKnownGoodIndex**: The index of the last known good audio device.
  - **Type**: `int32`
  - **Description**: Tracks the last valid device index.

- **bRequestDefault**: A flag indicating whether the default device should be requested.
  - **Type**: `bool`
  - **Description**: Determines if the system default device should be used.

## Methods
- **SetDiscreteOptionByIndex**: Sets the audio output device by its index.
  - **Parameters**:
    - `int32 Index`: The index of the desired audio output device.
  - **Description**: Overrides the base method to change the audio output device.

- **OnInitialized**: Called when the setting is initialized.
  - **Description**: Sets up callbacks and initializes the audio device list.

- **OnAudioOutputDevicesObtained**: Callback for when audio output devices are retrieved.
  - **Parameters**:
    - `const TArray<FAudioOutputDeviceInfo>& AvailableDevices`: The list of available devices.
  - **Description**: Updates the internal list of audio devices.

- **OnCompletedDeviceSwap**: Callback for when an audio device swap is completed.
  - **Parameters**:
    - `const FSwapAudioOutputResult& SwapResult`: The result of the device swap operation.
  - **Description**: Handles the result of the device swap.

- **DeviceAddedOrRemoved**: Handles when a device is added or removed.
  - **Parameters**:
    - `FString DeviceId`: The ID of the device that was added or removed.
  - **Description**: Updates the device list when changes occur.

- **DefaultDeviceChanged**: Handles when the system's default device changes.
  - **Parameters**:
    - `EAudioDeviceChangedRole InRole`: The role of the device change.
    - `FString DeviceId`: The ID of the new default device.
  - **Description**: Updates the default device ID.

## Events
- **DevicesObtainedCallback**: Triggered when audio devices are obtained.
  - **Type**: `FOnAudioOutputDevicesObtained`

- **DevicesSwappedCallback**: Triggered when an audio device swap is completed.
  - **Type**: `FOnCompletedDeviceSwap`

## Usage
1. Use this class to dynamically manage audio output devices in your game.
2. Override `SetDiscreteOptionByIndex` to implement custom behavior for device selection.
3. Use the callbacks to handle device changes and updates.

## Notes
- This class simplifies the management of audio output devices for games.
- It integrates with Unreal Engine's audio mixer system.

## See Also
- [UGameSettingValueDiscreteDynamic](https://docs.unrealengine.com/)
- [FAudioOutputDeviceInfo](https://docs.unrealengine.com/)
