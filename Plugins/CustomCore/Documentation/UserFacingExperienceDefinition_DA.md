# UUserFacingExperienceDefinition_DA

## Overview
`UUserFacingExperienceDefinition_DA` is a data asset that defines settings for user-facing experiences in the UI. It includes information about maps, gameplay experiences, and session settings.

## Features
- Defines map and experience IDs for gameplay sessions.
- Provides UI elements like titles, descriptions, and icons.
- Supports loading screen widgets and replay recording.
- Configures session settings like player count and extra arguments.

---

## Properties

### MapID
- **Type**: `FPrimaryAssetId`
- **Description**: The specific map to load.

### ExperienceID
- **Type**: `FPrimaryAssetId`
- **Description**: The gameplay experience to load.

### ExtraArgs
- **Type**: `TMap<FString, FString>`
- **Description**: Extra arguments passed as URL options to the game.

### TileTitle
- **Type**: `FText`
- **Description**: The primary title displayed in the UI.

### TileSubTitle
- **Type**: `FText`
- **Description**: The secondary title displayed in the UI.

### TileDescription
- **Type**: `FText`
- **Description**: The full description of the experience.

### TileIcon
- **Type**: `TObjectPtr<UTexture2D>`
- **Description**: The icon used in the UI.

### LoadingScreenWidget
- **Type**: `TSoftClassPtr<UUserWidget>`
- **Description**: The loading screen widget to show when loading into or out of the experience.

### bIsDefaultExperience
- **Type**: `bool`
- **Description**: Indicates if this is a default experience for quick play.

### bShowInFrontEnd
- **Type**: `bool`
- **Description**: Indicates if this experience should appear in the front-end UI.

### bRecordReplay
- **Type**: `bool`
- **Description**: Indicates if a replay should be recorded for this experience.

### MaxPlayerCount
- **Type**: `int32`
- **Description**: The maximum number of players for this session.

---

## Methods

### CreateHostingRequest
- **Description**: Creates a request object to start a session with the defined settings.
- **Parameters**:
  - `const UObject* WorldContextObject`: The world context.
- **Returns**: `UCommonSession_HostSessionRequest*` - The session request object.

---

## Usage
1. Create a `UUserFacingExperienceDefinition_DA` asset to define a user-facing experience.
2. Configure properties like `MapID`, `ExperienceID`, and `TileTitle` for the experience.
3. Use `CreateHostingRequest` to start a session with the defined settings.

## Notes
- This asset is designed for use in front-end UI and session management.
- Ensure that the `MapID` and `ExperienceID` are valid and properly configured.

## See Also
- [UExperienceDefinition_DA](ExperienceDefinition_DA.md)
- [UCommonSession_HostSessionRequest](https://docs.unrealengine.com/)
