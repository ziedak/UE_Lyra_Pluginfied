# UBaseGamePhaseAbility

## Overview

The `UBaseGamePhaseAbility` class is a specialized gameplay ability designed to manage and transition between game phases. It is part of the Unreal Engine Gameplay Ability System (GAS) and extends the functionality of `UBaseGameplayAbility`.

This class is used to define abilities that are tied to specific game phases. By associating a game phase with a gameplay tag, the system can manage transitions between phases, ensuring that sibling phases are canceled while allowing nested phases to coexist.

### Key Features
- Associates abilities with specific game phases using gameplay tags.
- Automatically cancels sibling phases when transitioning to a new phase.
- Supports nested phase behaviors, allowing parent and child phases to coexist.

## Properties

### `GamePhaseTag`
- **Type**: `FGameplayTag`
- **Category**: `Base|Game Phase`
- **Description**: Defines the game phase that this ability is associated with. For example, if the phase is `GamePhase.RoundStart`, activating this ability will cancel all sibling phases like `GamePhase.WaitingToStart`. Nested phases, such as `GamePhase.Playing.NormalPlay`, can coexist with their parent phase (`GamePhase.Playing`).

## Constructor

### `UBaseGamePhaseAbility(const FObjectInitializer& ObjectInitializer)`
- Initializes the ability with default policies:
  - `ReplicationPolicy`: `EGameplayAbilityReplicationPolicy::ReplicateNo`
  - `InstancingPolicy`: `EGameplayAbilityInstancingPolicy::InstancedPerActor`
  - `NetExecutionPolicy`: `EGameplayAbilityNetExecutionPolicy::ServerInitiated`
  - `NetSecurityPolicy`: `EGameplayAbilityNetSecurityPolicy::ServerOnly`

## Editor Validation (Editor-Only)

### `IsDataValid(FDataValidationContext& Context) const`
- **Returns**: `EDataValidationResult`
- **Description**: Validates the data for the ability in the editor. Ensures that the `GamePhaseTag` is set and valid. If the tag is missing, an error is added to the validation context.

## Usage

1. **Define a Game Phase Ability**:
   - Create a new class that inherits from `UBaseGamePhaseAbility`.
   - Assign a `GamePhaseTag` to specify the phase this ability represents.

2. **Activate the Ability**:
   - When the ability is activated, it transitions the game to the associated phase, canceling any conflicting sibling phases.

3. **Nested Phases**:
   - Use nested tags (e.g., `GamePhase.Playing.NormalPlay`) to create hierarchical phase structures. Activating a nested phase will not cancel its parent phase.

## Notes
- Ensure that the `GamePhaseTag` is properly defined in the gameplay tag configuration to avoid runtime errors.
- This class is optimized for use with the `UBaseGamePhaseSubsystem` for managing game phases.

## See Also
- [UBaseGamePhaseSubsystem](UBaseGamePhaseSubsystem.md)
- [BaseGameplayTags](BaseGameplayTags.md)
