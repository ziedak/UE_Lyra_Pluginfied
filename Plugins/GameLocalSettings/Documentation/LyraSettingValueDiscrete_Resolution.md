# ULyraSettingValueDiscrete_Resolution

## Overview
`ULyraSettingValueDiscrete_Resolution` is a class derived from `UGameSettingValueDiscrete`. It provides functionality for managing screen resolution settings in the game.

## Properties
- **Resolutions**: A list of resolutions for the current window mode.
  - **Type**: `TArray<TSharedPtr<FScreenResolutionEntry>>`
  - **Description**: Stores the available resolutions for the current window mode.

- **ResolutionsFullscreen**: A list of fullscreen resolutions.
  - **Type**: `TArray<TSharedPtr<FScreenResolutionEntry>>`
  - **Description**: Stores the available fullscreen resolutions.

- **ResolutionsWindowedFullscreen**: A list of windowed fullscreen resolutions.
  - **Type**: `TArray<TSharedPtr<FScreenResolutionEntry>>`
  - **Description**: Stores the available windowed fullscreen resolutions.

- **ResolutionsWindowed**: A list of windowed resolutions.
  - **Type**: `TArray<TSharedPtr<FScreenResolutionEntry>>`
  - **Description**: Stores the available windowed resolutions.

- **LastWindowMode**: The last selected window mode.
  - **Type**: `TOptional<EWindowMode::Type>`
  - **Description**: Stores the last selected window mode.

## Methods
- **StoreInitial**: Stores the initial state of the setting.
- **ResetToDefault**: Resets the setting to its default resolution.
- **RestoreToInitial**: Restores the setting to its initial resolution.
- **SetDiscreteOptionByIndex**: Sets the resolution option by its index.
- **GetDiscreteOptionIndex**: Retrieves the index of the currently selected resolution.
- **GetDiscreteOptions**: Retrieves the list of available resolution options.
- **OnInitialized**: Called when the setting is initialized.
- **OnDependencyChanged**: Called when a dependency of the setting changes.
- **InitializeResolutions**: Initializes the available resolutions.

## Nested Structs
- **FScreenResolutionEntry**: Represents a screen resolution entry.
  - **Properties**:
    - **Width**: The width of the resolution.
    - **Height**: The height of the resolution.
    - **RefreshRate**: The refresh rate of the resolution.
    - **OverrideText**: The display text for the resolution.
  - **Methods**:
    - **GetResolution**: Retrieves the resolution as an `FIntPoint`.
    - **GetDisplayText**: Retrieves the display text for the resolution.

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
1. Use `ULyraSettingValueDiscrete_Resolution` to manage screen resolution settings in your game.
2. Populate the resolution arrays with supported resolutions.
3. Use the provided methods to retrieve, update, and apply resolution settings.

## Notes
- This class is designed to simplify the management of screen resolution settings in games.
- It integrates with the `UGameSettingValueDiscrete` system for discrete options.

## See Also
- [UGameSettingValueDiscrete](https://docs.unrealengine.com/)
