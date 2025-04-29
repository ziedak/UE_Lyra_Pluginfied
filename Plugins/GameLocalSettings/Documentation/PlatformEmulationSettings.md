# UPlatformEmulationSettings

## Overview
`UPlatformEmulationSettings` is a class derived from `UDeveloperSettingsBackedByCVars`. It provides functionality for emulating platform-specific settings in the editor.

## Properties
- **AdditionalPlatformTraitsToEnable**: Platform traits to enable during emulation.
  - **Type**: `FGameplayTagContainer`
  - **Description**: Specifies additional platform traits to enable.

- **AdditionalPlatformTraitsToSuppress**: Platform traits to suppress during emulation.
  - **Type**: `FGameplayTagContainer`
  - **Description**: Specifies additional platform traits to suppress.

- **PretendPlatform**: The platform to emulate.
  - **Type**: `FName`
  - **Description**: Specifies the platform to emulate.

- **PretendBaseDeviceProfile**: The base device profile to emulate.
  - **Type**: `FName`
  - **Description**: Specifies the base device profile to emulate.

- **bApplyFrameRateSettingsInPIE**: Whether to apply frame rate settings in PIE.
  - **Type**: `bool`
  - **Description**: Determines if frame rate settings should be applied in Play-In-Editor (PIE) mode.

- **bApplyFrontEndPerformanceOptionsInPIE**: Whether to apply front-end performance options in PIE.
  - **Type**: `bool`
  - **Description**: Determines if front-end performance options should be applied in PIE mode.

- **bApplyDeviceProfilesInPIE**: Whether to apply device profiles in PIE.
  - **Type**: `bool`
  - **Description**: Determines if device profiles should be applied in PIE mode.

## Methods
- **GetPretendBaseDeviceProfile**: Retrieves the base device profile being emulated.
  - **Return Type**: `FName`

- **GetPretendPlatformName**: Retrieves the platform name being emulated.
  - **Return Type**: `FName`

- **OnPlayInEditorStarted** (Editor-only): Called when Play-In-Editor starts.
  - **Description**: Allows the subsystem to display notifications when cheats are active.

- **PostEditChangeProperty** (Editor-only): Called after a property is edited.
  - **Parameters**:
    - `FPropertyChangedEvent& PropertyChangedEvent`: The property change event.
  - **Description**: Handles changes to properties in the editor.

- **PostReloadConfig** (Editor-only): Called after the configuration is reloaded.
  - **Parameters**:
    - `FProperty* PropertyThatWasLoaded`: The property that was reloaded.
  - **Description**: Handles configuration reloads.

- **PostInitProperties** (Editor-only): Called after properties are initialized.
  - **Description**: Handles initialization of properties.

- **GetKnownPlatformIds**: Retrieves a list of known platform IDs.
  - **Return Type**: `TArray<FName>`

- **GetKnownDeviceProfiles**: Retrieves a list of known device profiles.
  - **Return Type**: `TArray<FName>`

- **PickReasonableBaseDeviceProfile**: Selects a reasonable base device profile for emulation.

## Usage
1. Use `UPlatformEmulationSettings` to configure platform emulation settings in the editor.
2. Set properties such as `PretendPlatform` and `PretendBaseDeviceProfile` to emulate specific platforms and profiles.
3. Enable or disable traits and performance options as needed for testing.

## Notes
- This class is designed for use in the editor to emulate platform-specific behavior.
- It integrates with Play-In-Editor (PIE) to apply emulation settings during testing.

## See Also
- [UDeveloperSettingsBackedByCVars](https://docs.unrealengine.com/)
