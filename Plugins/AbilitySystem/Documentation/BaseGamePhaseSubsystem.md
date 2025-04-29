# UBaseGamePhaseSubsystem

## Overview
`UBaseGamePhaseSubsystem` is a world subsystem designed to manage game phases using gameplay tags in a nested manner. It allows parent and child phases to coexist while ensuring sibling phases cannot be active simultaneously. This subsystem is ideal for managing hierarchical game states, such as gameplay modes or events.

## Features
- Supports nested game phases (e.g., `Game.Playing` and `Game.Playing.WarmUp`).
- Automatically ends sibling phases when a new phase starts.
- Provides callbacks for phase start and end events.
- Integrates with the Gameplay Ability System for phase management.

---

## Properties

### ActivePhaseMap
- **Type**: `TMap<FGameplayAbilitySpecHandle, FBaseGamePhaseEntry>`
- **Description**: Tracks active phases and their associated callbacks.

### PhaseStartObservers
- **Type**: `TArray<FPhaseObserver>`
- **Description**: Observers for phase start events.

### PhaseEndObservers
- **Type**: `TArray<FPhaseObserver>`
- **Description**: Observers for phase end events.

---

## Methods

### StartPhase
- **Description**: Starts a new game phase.
- **Parameters**:
  - `TSubclassOf<UBaseGamePhaseAbility> PhaseAbility`: The phase ability to start.
  - `FBaseGamePhaseDelegate PhaseEndedCallback`: Callback to execute when the phase ends.

### WhenPhaseStartsOrIsActive
- **Description**: Registers a callback for when a phase starts or is already active.
- **Parameters**:
  - `FGameplayTag PhaseTag`: The tag of the phase to observe.
  - `EPhaseTagMatchType MatchType`: The match type for the phase tag.
  - `FBaseGamePhaseTagDelegate WhenPhaseActive`: The callback to execute.

### WhenPhaseEnds
- **Description**: Registers a callback for when a phase ends.
- **Parameters**:
  - `FGameplayTag PhaseTag`: The tag of the phase to observe.
  - `EPhaseTagMatchType MatchType`: The match type for the phase tag.
  - `FBaseGamePhaseTagDelegate WhenPhaseEnd`: The callback to execute.

### IsPhaseActive
- **Description**: Checks if a phase is currently active.
- **Parameters**:
  - `const FGameplayTag& PhaseTag`: The tag of the phase to check.
- **Returns**: `bool`

### K2_StartPhase
- **Description**: Blueprint-accessible method to start a phase.
- **Parameters**:
  - `TSubclassOf<UBaseGamePhaseAbility> Phase`: The phase ability to start.
  - `FBaseGamePhaseDynamicDelegate PhaseEnded`: Callback to execute when the phase ends.

### K2_WhenPhaseStartsOrIsActive
- **Description**: Blueprint-accessible method to register a callback for when a phase starts or is active.
- **Parameters**:
  - `FGameplayTag PhaseTag`: The tag of the phase to observe.
  - `EPhaseTagMatchType MatchType`: The match type for the phase tag.
  - `FBaseGamePhaseTagDynamicDelegate WhenPhaseActive`: The callback to execute.

### K2_WhenPhaseEnds
- **Description**: Blueprint-accessible method to register a callback for when a phase ends.
- **Parameters**:
  - `FGameplayTag PhaseTag`: The tag of the phase to observe.
  - `EPhaseTagMatchType MatchType`: The match type for the phase tag.
  - `FBaseGamePhaseTagDynamicDelegate WhenPhaseEnd`: The callback to execute.

---

## Usage
1. Use `StartPhase` to begin a new game phase.
2. Register callbacks with `WhenPhaseStartsOrIsActive` and `WhenPhaseEnds`.
3. Use `IsPhaseActive` to check the status of a phase.
4. Use the `K2_` prefixed methods for Blueprint integration.

## Notes
- Ensure that gameplay tags for phases are unique and well-structured to avoid conflicts.
- Use nested phases to represent hierarchical game states effectively.
- This subsystem integrates seamlessly with `UBaseGamePhaseAbility` for phase-specific logic.

## See Also
- [UBaseGamePhaseAbility](UBaseGamePhaseAbility.md)
- [Gameplay Ability System](https://docs.unrealengine.com/)
