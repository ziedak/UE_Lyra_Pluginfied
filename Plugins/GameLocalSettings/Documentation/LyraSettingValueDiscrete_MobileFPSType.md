# ULyraSettingValueDiscrete_MobileFPSType

## Overview
`ULyraSettingValueDiscrete_MobileFPSType` is a class derived from `UGameSettingValueDiscrete`. It provides functionality for managing discrete FPS settings for mobile devices.

## Properties
- **InitialValue**: The initial FPS value.
  - **Type**: `int32`
  - **Description**: Stores the FPS value when the setting is initialized.

- **FPSOptions**: A sorted map of FPS options.
  - **Type**: `TSortedMap<int32, FText>`
  - **Description**: Maps FPS values to their corresponding display text.

## Methods
- **StoreInitial**: Stores the initial state of the setting.
  - **Description**: Called to save the initial FPS value for later restoration.

- **ResetToDefault**: Resets the setting to its default FPS value.
  - **Description**: Restores the FPS setting to its predefined default state.

- **RestoreToInitial**: Restores the setting to its initial FPS value.
  - **Description**: Reverts the FPS setting to the value stored during initialization.

- **SetDiscreteOptionByIndex**: Sets the FPS option by its index.
  - **Parameters**:
    - `int32 Index`: The index of the FPS option to set.
  - **Description**: Updates the current FPS setting based on the provided index.

- **GetDiscreteOptionIndex**: Retrieves the index of the currently selected FPS option.
  - **Return Type**: `int32`
  - **Description**: Returns the index of the currently active FPS option.

- **GetDiscreteOptions**: Retrieves the list of available FPS options.
  - **Return Type**: `TArray<FText>`
  - **Description**: Provides a list of FPS options as localized text.

- **OnInitialized**: Called when the setting is initialized.
  - **Description**: Performs setup tasks when the setting is first created.

- **GetValue**: Retrieves the current FPS value.
  - **Return Type**: `int32`
  - **Description**: Returns the current FPS limit.

- **SetValue**: Sets a new FPS value.
  - **Parameters**:
    - `int32 NewLimitFPS`: The new FPS limit to set.
    - `EGameSettingChangeReason InReason`: The reason for the change.
  - **Description**: Updates the FPS limit and triggers any necessary updates.

- **GetDefaultFPS**: Retrieves the default FPS value.
  - **Return Type**: `int32`
  - **Description**: Returns the default FPS limit.

- **MakeLimitString**: Creates a display string for an FPS value.
  - **Parameters**:
    - `int32 Number`: The FPS value to convert.
  - **Return Type**: `FText`
  - **Description**: Converts an FPS value into a localized text string.

## Events
- None.

## Overrides
- **UGameSettingValueDiscrete**:
  - `SetDiscreteOptionByIndex`
  - `GetDiscreteOptionIndex`
  - `GetDiscreteOptions`

- **UGameSettingValue**:
  - `StoreInitial`
  - `ResetToDefault`
  - `RestoreToInitial`
  - `OnInitialized`

## Usage
1. Use `ULyraSettingValueDiscrete_MobileFPSType` to manage FPS settings for mobile devices.
2. Populate the `FPSOptions` map with supported FPS values and their corresponding display text.
3. Use the provided methods to retrieve, update, and apply FPS settings.

## Notes
- This class is designed to simplify the management of FPS settings for mobile platforms.
- It integrates with the `UGameSettingValueDiscrete` system for discrete options.

## See Also
- [UGameSettingValueDiscrete](https://docs.unrealengine.com/)
