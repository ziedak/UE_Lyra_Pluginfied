# ABaseWorldSettings

## Overview
`ABaseWorldSettings` extends `AWorldSettings` to provide additional functionality for managing default gameplay experiences and editor-specific settings.

## Features
- Defines the default gameplay experience for the map.
- Provides editor-only settings for standalone net mode.

---

## Properties

### DefaultGameplayExperience
- **Type**: `TSoftClassPtr<UExperienceDefinition_DA>`
- **Description**: The default experience to use when a server opens this map, unless overridden by a user-facing experience.

### ForceStandaloneNetMode
- **Type**: `bool`
- **Editor Only**: Indicates if the level is part of a standalone experience, forcing the net mode to standalone during PIE.

---

## Methods

### GetDefaultGameplayExperience
- **Description**: Returns the default gameplay experience for the map.
- **Returns**: `FPrimaryAssetId` - The asset ID of the default experience.

### CheckForErrors
- **Editor Only**: Checks for errors in the world settings.
- **Overrides**: `AWorldSettings::CheckForErrors`.

---

## Usage
1. Use `ABaseWorldSettings` as the default world settings class for your maps.
2. Configure `DefaultGameplayExperience` to specify the experience for the map.
3. Use `ForceStandaloneNetMode` for front-end or standalone experiences during PIE.

## Notes
- This class integrates with the experience system to determine the gameplay experience for the map.
- Ensure that the `DefaultGameplayExperience` is valid and properly configured.

## See Also
- [UExperienceDefinition_DA](ExperienceDefinition_DA.md)
- [AWorldSettings](https://docs.unrealengine.com/)
