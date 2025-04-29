# ABaseGameMode

## Overview
`ABaseGameMode` is the base game mode class used in this project. It extends `AModularGameModeBase` and provides additional functionality for managing player initialization, spawning, and game state.

## Features
- Handles player initialization and spawning logic.
- Provides hooks for experience loading and validation.
- Manages dedicated server login and session hosting.
- Customizable player restart and spawn behavior.

---

## Properties

### OnGameModePlayerInitialized
- **Type**: `FOnLyraGameModePlayerInitialized`
- **Description**: A multicast delegate triggered when a player or bot is initialized.

---

## Methods

### InitGame
- **Description**: Initializes the game with the specified map and options.
- **Parameters**:
  - `const FString& MapName`: The name of the map.
  - `const FString& Options`: Game options.
  - `FString& ErrorMessage`: Error message if initialization fails.

### HandleStartingNewPlayer_Implementation
- **Description**: Handles logic for starting a new player.
- **Parameters**:
  - `APlayerController* NewPlayer`: The new player controller.

### ChoosePlayerStart_Implementation
- **Description**: Chooses a player start location.
- **Parameters**:
  - `AController* Player`: The player controller.
- **Returns**: `AActor*` - The chosen player start.

### RequestPlayerRestartNextFrame
- **Description**: Requests a player or bot restart on the next frame.
- **Parameters**:
  - `AController* Controller`: The controller to restart.
  - `bool bForceReset`: Whether to force a reset.

### ControllerCanRestart
- **Description**: Determines if a controller can restart.
- **Parameters**:
  - `AController* Controller`: The controller to check.
- **Returns**: `bool` - Whether the controller can restart.

### OnExperienceLoaded
- **Description**: Called when the experience is loaded.
- **Parameters**:
  - `const UExperienceDefinition_DA* CurrentExperience`: The loaded experience.

---

## Usage
1. Use `ABaseGameMode` as the base class for your custom game mode.
2. Override methods like `InitGame` and `HandleStartingNewPlayer_Implementation` to customize game logic.
3. Use `RequestPlayerRestartNextFrame` to handle player respawns.

## Notes
- This class integrates with the experience system to load and validate game experiences.
- It is designed to work with both player and bot controllers.

## See Also
- [UExperienceDefinition_DA](ExperienceDefinition_DA.md)
- [AModularGameModeBase](https://docs.unrealengine.com/)
