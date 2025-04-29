# ULyraSettingValueDiscrete_Language

## Overview
`ULyraSettingValueDiscrete_Language` is a class derived from `UGameSettingValueDiscrete`. It provides functionality for managing discrete language settings in the game.

## Properties
- **AvailableCultureNames**: A list of available culture names.
  - **Type**: `TArray<FString>`
  - **Description**: Stores the culture names that can be selected as language options.

## Methods
- **StoreInitial**: Stores the initial state of the setting.
  - **Description**: Called to save the initial value of the setting for later restoration.

- **ResetToDefault**: Resets the setting to its default value.
  - **Description**: Restores the setting to its predefined default state.

- **RestoreToInitial**: Restores the setting to its initial value.
  - **Description**: Reverts the setting to the value stored during initialization.

- **SetDiscreteOptionByIndex**: Sets the language option by its index.
  - **Parameters**:
    - `int32 Index`: The index of the language option to set.
  - **Description**: Updates the current language setting based on the provided index.

- **GetDiscreteOptionIndex**: Retrieves the index of the currently selected language option.
  - **Return Type**: `int32`
  - **Description**: Returns the index of the currently active language option.

- **GetDiscreteOptions**: Retrieves the list of available language options.
  - **Return Type**: `TArray<FText>`
  - **Description**: Provides a list of language options as localized text.

- **OnInitialized**: Called when the setting is initialized.
  - **Description**: Performs setup tasks when the setting is first created.

- **OnApply**: Applies the current setting.
  - **Description**: Executes logic to apply the selected language setting.

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
  - `OnApply`

## Usage
1. Use `ULyraSettingValueDiscrete_Language` to manage language settings in your game.
2. Populate the `AvailableCultureNames` array with supported culture names.
3. Use the provided methods to retrieve, update, and apply language settings.

## Notes
- This class is designed to simplify the management of language settings in games.
- It integrates with the `UGameSettingValueDiscrete` system for discrete options.

## See Also
- [UGameSettingValueDiscrete](https://docs.unrealengine.com/)
