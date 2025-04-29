# UPlayerSpawningManagerComponent

## Overview
`UPlayerSpawningManagerComponent` is a component that manages player spawning logic, including selecting player start locations and handling respawn events.

## Features
- Provides utility for finding unoccupied player start locations.
- Customizable player start selection logic.
- Handles player respawn events with Blueprint support.

---

## Properties

### CachedPlayerStarts
- **Type**: `TArray<TWeakObjectPtr<APlayerStart>>`
- **Description**: A transient array of cached player start locations.

---

## Methods

### ChoosePlayerStart
- **Description**: Proxies the call from `ABaseGameMode` to select a player start location.
- **Parameters**:
  - `AController* Player`: The player controller.
- **Returns**: `AActor*` - The chosen player start.

### ControllerCanRestart
- **Description**: Determines if a controller can restart.
- **Parameters**:
  - `AController* Player`: The player controller.
- **Returns**: `bool` - Whether the controller can restart.

### FinishRestartPlayer
- **Description**: Finalizes the player restart process.
- **Parameters**:
  - `AController* NewPlayer`: The player controller.
  - `const FRotator& StartRotation`: The rotation to apply to the player.

### GetFirstRandomUnoccupiedPlayerStart
- **Description**: Finds the first unoccupied player start location.
- **Parameters**:
  - `AController* Controller`: The player controller.
  - `const TArray<APlayerStart*>& FoundStartPoints`: The list of available player start points.
- **Returns**: `APlayerStart*` - The first unoccupied player start.

### OnFinishRestartPlayer (BlueprintImplementableEvent)
- **Description**: Blueprint event triggered when a player restart is finished.
- **Parameters**:
  - `AController* Player`: The player controller.
  - `const FRotator& StartRotation`: The rotation applied to the player.

---

## Usage
1. Attach `UPlayerSpawningManagerComponent` to your game state or game mode.
2. Override `OnChoosePlayerStart` to customize player start selection logic.
3. Use `OnFinishRestartPlayer` in Blueprints to handle post-respawn logic.

## Notes
- This component is designed to work with `ABaseGameMode` for managing player spawning and respawning.
- Ensure that player start locations are properly configured in the level.

## See Also
- [ABaseGameMode](BaseGameMode.md)
- [APlayerStart](https://docs.unrealengine.com/)
