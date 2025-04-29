# FGameplayAbilityData

## Overview
`FGameplayAbilityData` defines data for granting gameplay abilities. It is used to specify the ability, its level, and the input tag associated with it.

## Properties
- **Ability**: The gameplay ability to grant.
- **AbilityLevel**: The level of the ability.
- **InputTag**: The input tag associated with the ability.

## Methods
- None.

## Events
- None.

## Overrides
- None.

## Usage
1. Use this structure to define abilities in an ability set.
2. Assign the `Ability`, `AbilityLevel`, and `InputTag` properties to configure the ability.

## Notes
- Ensure that the `Ability` is a valid subclass of `UBaseGameplayAbility`.
- The `InputTag` should correspond to a tag defined in the gameplay tag configuration.

## See Also
- [UBaseAbilitySet](UBaseAbilitySet.md)
- [UBaseGameplayAbility](UBaseGameplayAbility.md)
- [BaseGameplayTags](BaseGameplayTags.md)
