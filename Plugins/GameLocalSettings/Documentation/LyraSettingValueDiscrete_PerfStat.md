# ULyraSettingValueDiscrete_PerfStat

## Overview
`ULyraSettingValueDiscrete_PerfStat` is a class derived from `UGameSettingValueDiscrete`. It provides functionality for managing performance statistics display settings in the game.

## Properties
- **Options**: A list of performance stat options.
  - **Type**: `TArray<FText>`
  - **Description**: Stores the available performance stat options.

- **DisplayModes**: A list of display modes for performance stats.
  - **Type**: `TArray<EStatDisplayMode>`
  - **Description**: Stores the available display modes for performance stats.

- **StatToDisplay**: The performance stat to display.
  - **Type**: `EDisplayablePerformanceStat`
  - **Description**: Specifies which performance stat is currently being displayed.

- **InitialMode**: The initial display mode.
  - **Type**: `EStatDisplayMode`
  - **Description**: Stores the display mode when the setting is initialized.

## Methods
- **SetStat**: Sets the performance stat to display.
  - **Parameters**:
    - `EDisplayablePerformanceStat InStat`: The performance stat to display.
  - **Description**: Updates the stat to be displayed.

- **StoreInitial**: Stores the initial state of the setting.
  - **Description**: Called to save the initial value of the setting for later restoration.

- **ResetToDefault**: Resets the setting to its default value.
  - **Description**: Restores the setting to its predefined default state.

- **RestoreToInitial**: Restores the setting to its initial value.
  - **Description**: Reverts the setting to the value stored during initialization.

- **SetDiscreteOptionByIndex**: Sets the performance stat option by its index.
  - **Parameters**:
    - `int32 Index`: The index of the performance stat option to set.
  - **Description**: Updates the current performance stat setting based on the provided index.

- **GetDiscreteOptionIndex**: Retrieves the index of the currently selected performance stat option.
  - **Return Type**: `int32`
  - **Description**: Returns the index of the currently active performance stat option.

- **GetDiscreteOptions**: Retrieves the list of available performance stat options.
  - **Return Type**: `TArray<FText>`
  - **Description**: Provides a list of performance stat options as localized text.

- **OnInitialized**: Called when the setting is initialized.
  - **Description**: Performs setup tasks when the setting is first created.

- **AddMode**: Adds a new display mode for performance stats.
  - **Parameters**:
    - `FText&& Label`: The label for the display mode.
    - `EStatDisplayMode Mode`: The display mode to add.
  - **Description**: Adds a new display mode to the list of available modes.

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
1. Use `ULyraSettingValueDiscrete_PerfStat` to manage performance stat display settings in your game.
2. Populate the `Options` and `DisplayModes` arrays with supported performance stats and display modes.
3. Use the provided methods to retrieve, update, and apply performance stat settings.

## Notes
- This class is designed to simplify the management of performance stat display settings in games.
- It integrates with the `UGameSettingValueDiscrete` system for discrete options.

## See Also
- [UGameSettingValueDiscrete](https://docs.unrealengine.com/)
