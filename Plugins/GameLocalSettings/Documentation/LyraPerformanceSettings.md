# ULyraPerformanceSettings

## Overview
`ULyraPerformanceSettings` is a class derived from `UDeveloperSettingsBackedByCVars`. It provides project-specific performance profile settings, including frame rate limits and performance stats.

## Properties
- **DesktopFrameRateLimits**: List of frame rates available for desktop platforms.
  - **Type**: `TArray<int32>`
  - **Description**: Specifies the frame rate options for desktop platforms.

- **UserFacingPerformanceStats**: Performance stats that can be enabled by the user.
  - **Type**: `TArray<FLyraPerformanceStatGroup>`
  - **Description**: Contains groups of performance stats that users can enable in the settings.

- **PerPlatformSettings**: Platform-specific settings.
  - **Type**: `FPerPlatformSettings`
  - **Description**: Exposes platform-specific settings for editing in the project settings.

## Methods
- **ULyraPerformanceSettings**: Constructor for the class.
  - **Description**: Initializes the performance settings.

## Nested Structs
### FLyraQualityDeviceProfileVariant
- **Description**: Describes a platform-specific device profile variant.
- **Properties**:
  - `DisplayName`: Display name for the device profile variant.
  - `DeviceProfileSuffix`: Suffix to append to the base device profile name.
  - `MinRefreshRate`: Minimum refresh rate required to enable this mode.

### FLyraPerformanceStatGroup
- **Description**: Describes a set of performance stats that can be enabled.
- **Properties**:
  - `VisibilityQuery`: Query to determine if the stats can be shown.
  - `AllowedStats`: Set of stats allowed if the query passes.

## Usage
1. Use `ULyraPerformanceSettings` to configure performance settings for your project.
2. Populate `DesktopFrameRateLimits` and `UserFacingPerformanceStats` with appropriate values.
3. Use platform-specific INI files to configure `PerPlatformSettings`.

## Notes
- This class is designed to simplify the management of performance settings across platforms.
- It integrates with the Unreal Engine settings system for easy configuration.

## See Also
- [UDeveloperSettingsBackedByCVars](https://docs.unrealengine.com/)
