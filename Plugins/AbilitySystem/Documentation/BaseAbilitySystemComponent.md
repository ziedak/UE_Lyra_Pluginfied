# UBaseAbilitySystemComponent

## Overview
`UBaseAbilitySystemComponent` extends the `UAbilitySystemComponent` to provide additional functionality for managing abilities, input, and activation groups in the GAS (Gameplay Ability System).

## Features
- Handles ability input processing (pressed, held, released).
- Manages activation groups for exclusive and independent abilities.
- Integrates with global systems for applying abilities and effects.
- Provides utility methods for managing dynamic tags and canceling abilities.

---

## Properties

### ActivationGroupCounts
- **Type**: `int32[3]`
- **Description**: Tracks the count of abilities in each activation group (Independent, Exclusive_Replaceable, Exclusive_Blocking).

### InputPressedSpecHandlesList
- **Type**: `TSet<FGameplayAbilitySpecHandle>`
- **Description**: Stores ability handles for abilities whose input was pressed this frame.

### InputHeldSpecHandlesList
- **Type**: `TSet<FGameplayAbilitySpecHandle>`
- **Description**: Stores ability handles for abilities whose input is currently held.

### InputReleasedSpecHandlesList
- **Type**: `TSet<FGameplayAbilitySpecHandle>`
- **Description**: Stores ability handles for abilities whose input was released this frame.

---

## Methods

### InitAbilityActorInfo
- **Description**: Initializes the ability actor info for the component.
- **Parameters**:
  - `AActor* InOwnerActor`: The owner actor of the component.
  - `AActor* InAvatarActor`: The avatar actor associated with the component.

### ProcessAbilityInput
- **Description**: Processes ability input (pressed, held, released) and attempts to activate abilities.
- **Parameters**:
  - `float DeltaTime`: The time elapsed since the last frame.
  - `bool bGamePaused`: Whether the game is currently paused.

### CancelAbilitiesByFunc
- **Description**: Cancels abilities based on a custom predicate function.
- **Parameters**:
  - `TShouldCancelAbilityFunc ShouldCancelFunc`: A function to determine which abilities to cancel.
  - `bool bReplicateCancelAbility`: Whether to replicate the cancel event.

### AddDynamicTagGameplayEffect
- **Description**: Adds a dynamic tag gameplay effect to the component.
- **Parameters**:
  - `TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect`: The gameplay effect to apply.
  - `FGameplayTag Tag`: The tag to add dynamically.

### RemoveDynamicTagGameplayEffect
- **Description**: Removes a dynamic tag gameplay effect from the component.
- **Parameters**:
  - `TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect`: The gameplay effect to remove.
  - `FGameplayTag Tag`: The tag to remove dynamically.

### SetAbilityInputTagPressed
- **Description**: Marks an input tag as pressed.
- **Parameters**:
  - `FGameplayTag InputTag`: The input tag to mark as pressed.

### SetAbilityInputTagReleased
- **Description**: Marks an input tag as released.
- **Parameters**:
  - `FGameplayTag InputTag`: The input tag to mark as released.

---

## Usage
1. Attach `UBaseAbilitySystemComponent` to an actor to manage its abilities.
2. Use `SetAbilityInputTagPressed` and `SetAbilityInputTagReleased` to handle input.
3. Call `ProcessAbilityInput` every frame to process input and activate abilities.
4. Use `AddDynamicTagGameplayEffect` and `RemoveDynamicTagGameplayEffect` to manage dynamic tags.

## Notes
- This component is designed to integrate with the `UGlobalGasWorldSubsystem` for global ability and effect management.
- Ensure that input tags are properly mapped to gameplay abilities for smooth input handling.

## See Also
- [UBaseGameplayAbility](UBaseGameplayAbility.md)
- [UGlobalGasWorldSubsystem](GlobalGasWorldSubsystem.md)
- [BaseGameplayTags](BaseGameplayTags.md)
