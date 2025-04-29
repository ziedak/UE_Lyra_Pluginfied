# UExperienceManagerComponent

## Overview
`UExperienceManagerComponent` is a component that manages the loading, activation, and deactivation of gameplay experiences. It extends `UGameStateComponent` and implements `ILoadingProcessInterface`.

## Features
- Handles experience loading and state transitions.
- Manages game feature plugins and scalability settings.
- Provides high-priority, normal, and low-priority experience load callbacks.
- Supports testing delays and partial load states for debugging.

---

## Properties

### CurrentExperience
- **Type**: `TObjectPtr<const UExperienceDefinition_DA>`
- **Description**: The currently loaded experience.

### LoadState
- **Type**: `EExperienceLoadState`
- **Description**: The current load state of the experience.

### OnExperienceLoaded_HighPriority
- **Type**: `FOnExperienceLoaded`
- **Description**: Delegate called when the experience is loaded (high priority).

### OnExperienceLoaded
- **Type**: `FOnExperienceLoaded`
- **Description**: Delegate called when the experience is loaded (normal priority).

### OnExperienceLoaded_LowPriority
- **Type**: `FOnExperienceLoaded`
- **Description**: Delegate called when the experience is loaded (low priority).

---

## Methods

### SetCurrentExperience
- **Description**: Sets the current experience to load.
- **Parameters**:
  - `const FPrimaryAssetId& ExperienceId`: The ID of the experience to load.

### CallOrRegister_OnExperienceLoaded_HighPriority
- **Description**: Registers a high-priority callback for when the experience is loaded.
- **Parameters**:
  - `FOnExperienceLoaded::FDelegate&& Delegate`: The delegate to register.

### CallOrRegister_OnExperienceLoaded
- **Description**: Registers a normal-priority callback for when the experience is loaded.
- **Parameters**:
  - `FOnExperienceLoaded::FDelegate&& Delegate`: The delegate to register.

### CallOrRegister_OnExperienceLoaded_LowPriority
- **Description**: Registers a low-priority callback for when the experience is loaded.
- **Parameters**:
  - `FOnExperienceLoaded::FDelegate&& Delegate`: The delegate to register.

### GetCurrentExperienceChecked
- **Description**: Returns the current experience if it is fully loaded.
- **Returns**: `const UExperienceDefinition_DA*` - The current experience.

### IsExperienceLoaded
- **Description**: Checks if the experience is fully loaded.
- **Returns**: `bool` - Whether the experience is loaded.

---

## Usage
1. Attach `UExperienceManagerComponent` to your game state to manage experiences.
2. Use `SetCurrentExperience` to specify the experience to load.
3. Register callbacks using `CallOrRegister_OnExperienceLoaded` methods to handle experience load events.

## Notes
- This component integrates with game feature plugins and scalability settings.
- It supports testing delays and partial load states for debugging purposes.

## See Also
- [UExperienceDefinition_DA](ExperienceDefinition_DA.md)
- [UGameFeatureAction](GameFeatureAction.md)
