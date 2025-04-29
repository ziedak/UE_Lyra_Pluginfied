# ABaseGameState

## Overview
`ABaseGameState` extends `AModularGameStateBase` to provide additional functionality for managing server FPS, ability systems, and client notifications.

## Features
- Integrates with the Gameplay Ability System (GAS).
- Manages server FPS and replicates it to clients.
- Provides methods for sending reliable and unreliable messages to clients.

---

## Properties

### ServerFPS
- **Type**: `float`
- **Description**: The server's FPS, replicated to clients.

### AbilitySystemComponent
- **Type**: `TObjectPtr<UAbilitySystemComponent>`
- **Description**: The ability system component for game-wide gameplay cues.

### ExperienceManagerComponent
- **Type**: `TObjectPtr<UExperienceManagerComponent>`
- **Description**: Manages the current gameplay experience.

---

## Methods

### SetServerFPS
- **Description**: Sets the server's FPS.
- **Parameters**:
  - `float NewServerFPS`: The new server FPS value.

### GetServerFPS
- **Description**: Gets the server's FPS.
- **Returns**: `float` - The current server FPS.

### MulticastMessageToClients_Unreliable
- **Description**: Sends an unreliable message to all clients.
- **Parameters**:
  - `const FVerbMessage Message`: The message to send.

### MulticastMessageToClients_Reliable
- **Description**: Sends a reliable message to all clients.
- **Parameters**:
  - `const FVerbMessage Message`: The message to send.

### GetAbilitySystemComponent
- **Description**: Returns the ability system component.
- **Returns**: `UAbilitySystemComponent*` - The ability system component.

---

## Usage
1. Use `ABaseGameState` as the base class for your game state.
2. Call `SetServerFPS` to update the server FPS and replicate it to clients.
3. Use `MulticastMessageToClients_Unreliable` and `MulticastMessageToClients_Reliable` to send notifications to clients.

## Notes
- This class integrates with the Gameplay Ability System for managing game-wide gameplay cues.
- Ensure that the `ExperienceManagerComponent` is properly configured for managing gameplay experiences.

## See Also
- [UAbilitySystemComponent](https://docs.unrealengine.com/)
- [UExperienceManagerComponent](ExperienceManagerComponent.md)
