# ULyraSettingValueDiscrete_OverallQuality

## Overview
`ULyraSettingValueDiscrete_OverallQuality` is a class derived from `UGameSettingValueDiscrete`. It provides functionality for managing overall quality settings in the game.

## Properties
- **Options**: A list of quality options.
  - **Type**: `TArray<FText>`
  - **Description**: Stores the available quality options.

- **OptionsWithCustom**: A list of quality options including a custom option.
  - **Type**: `TArray<FText>`
  - **Description**: Stores the available quality options with an additional custom option.

## Methods
- **StoreInitial**: Stores the initial state of the setting.
  - **Description**: Called to save the initial value of the setting for later restoration.

- **ResetToDefault**: Resets the setting to its default value.
  - **Description**: Restores the setting to its predefined default state.

- **RestoreToInitial**: Restores the setting to its initial value.
  - **Description**: Reverts the setting to the value stored during initialization.

- **SetDiscreteOptionByIndex**: Sets the quality option by its index.
  - **Parameters**:
    - `int32 Index`: The index of the quality option to set.
  - **Description**: Updates the current quality setting based on the provided index.

- **GetDiscreteOptionIndex**: Retrieves the index of the currently selected quality option.
  - **Return Type**: `int32`
  - **Description**: Returns the index of the currently active quality option.

- **GetDiscreteOptions**: Retrieves the list of available quality options.
  - **Return Type**: `TArray<FText>`
  - **Description**: Provides a list of quality options as localized text.

- **OnInitialized**: Called when the setting is initialized.
  - **Description**: Performs setup tasks when the setting is first created.

- **GetCustomOptionIndex**: Retrieves the index of the custom quality option.
  - **Return Type**: `int32`
  - **Description**: Returns the index of the custom quality option.

- **GetOverallQualityLevel**: Retrieves the current overall quality level.
  - **Return Type**: `int32`
  - **Description**: Returns the current overall quality level.

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
1. Use `ULyraSettingValueDiscrete_OverallQuality` to manage overall quality settings in your game.
2. Populate the `Options` and `OptionsWithCustom` arrays with supported quality levels.
3. Use the provided methods to retrieve, update, and apply quality settings.

## Notes
- This class is designed to simplify the management of overall quality settings in games.
- It integrates with the `UGameSettingValueDiscrete` system for discrete options.

## See Also
- [UGameSettingValueDiscrete](https://docs.unrealengine.com/)
