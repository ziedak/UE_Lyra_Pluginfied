# FGrantedHandlesData

## Overview
`FGrantedHandlesData` stores handles for abilities, effects, and attribute sets granted by an ability set. It provides methods to manage these handles and remove them from the ability system.

## Properties
- **AbilitySpecHandles**: A list of handles for granted abilities.
  - **Type**: `TArray<FGameplayAbilitySpecHandle>`
  - **Description**: Stores the handles of all abilities granted by the ability set.
- **GameplayEffectHandles**: A list of handles for granted gameplay effects.
  - **Type**: `TArray<FActiveGameplayEffectHandle>`
  - **Description**: Stores the handles of all gameplay effects applied by the ability set.
- **GrantedAttributeSets**: A list of pointers to granted attribute sets.
  - **Type**: `TArray<TObjectPtr<UAttributeSet>>`
  - **Description**: Stores pointers to all attribute sets granted by the ability set.

## Methods
- **AddAbilitySpecHandle**: Adds a handle for a granted ability.
  - **Parameters**:
    - `const FGameplayAbilitySpecHandle& Handle`: The handle of the granted ability.
  - **Description**: Adds the specified ability handle to the `AbilitySpecHandles` list.

- **AddGameplayEffectHandle**: Adds a handle for a granted gameplay effect.
  - **Parameters**:
    - `const FActiveGameplayEffectHandle& Handle`: The handle of the granted gameplay effect.
  - **Description**: Adds the specified effect handle to the `GameplayEffectHandles` list.

- **AddAttributeSet**: Adds a granted attribute set.
  - **Parameters**:
    - `UAttributeSet* Set`: A pointer to the granted attribute set.
  - **Description**: Adds the specified attribute set to the `GrantedAttributeSets` list.

- **TakeFromAbilitySystem**: Removes all granted items from the ability system.
  - **Parameters**:
    - `UBaseAbilitySystemComponent* BaseAsc`: The ability system component from which the granted items will be removed.
  - **Description**: Iterates through all stored handles and removes the corresponding abilities, effects, and attributes from the specified ability system component.

## Events
- None.

## Overrides
- None.

## Usage
1. Use this structure to track handles for abilities, effects, and attributes granted by an ability set.
2. Call `TakeFromAbilitySystem` to remove all granted items when they are no longer needed.

## Notes
- Ensure that the ability system component is valid before calling `TakeFromAbilitySystem`.
- This structure is typically used internally by the `UBaseAbilitySet`.

## See Also
- [UBaseAbilitySet](UBaseAbilitySet.md)
- [FGameplayAbilityData](FGameplayAbilityData.md)
- [FGameplayEffectData](GameplayEffectData.md)
