# FGameplayEffectData

## Overview
`FGameplayEffectData` defines data for granting gameplay effects. It specifies the effect class and its level, allowing for flexible configuration of effects applied to actors.

## Properties
- **GameplayEffect**: The gameplay effect to grant.
  - **Type**: `TSubclassOf<UGameplayEffect>`
  - **Description**: A reference to the gameplay effect class that will be applied.
- **EffectLevel**: The level of the gameplay effect.
  - **Type**: `float`
  - **Description**: Determines the strength or magnitude of the gameplay effect.

## Methods
- None.

## Events
- None.

## Overrides
- None.

## Usage
1. Use this structure to define gameplay effects in an ability set.
2. Assign the `GameplayEffect` property to specify the effect to be applied.
3. Set the `EffectLevel` property to control the strength of the effect.
4. Include this structure in the `GrantedGameplayEffects` list of a `UBaseAbilitySet`.

## Notes
- Ensure that the `GameplayEffect` is a valid subclass of `UGameplayEffect`.
- The `EffectLevel` should be set based on the desired strength of the effect.
- This structure is typically used in conjunction with `UBaseAbilitySet` to apply effects during gameplay.

## See Also
- [UBaseAbilitySet](UBaseAbilitySet.md)
- [FGrantedHandlesData](FGrantedHandlesData.md)
- [BaseGameplayTags](BaseGameplayTags.md)
