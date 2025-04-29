# UBaseAbilitySet

## Overview
`UBaseAbilitySet` is a non-mutable data asset used to grant gameplay abilities, gameplay effects, and attribute sets. It provides a centralized way to define and manage these elements for actors in the game.

## Properties
- **GrantedGameplayAbilities**: A list of gameplay abilities to grant when this ability set is applied.
  - **Type**: `TArray<FGameplayAbilityData>`
  - **Description**: Contains data for each ability, including the ability class, level, and input tag.
- **GrantedGameplayEffects**: A list of gameplay effects to grant when this ability set is applied.
  - **Type**: `TArray<FGameplayEffectData>`
  - **Description**: Contains data for each effect, including the effect class and level.
- **GrantedAttributes**: A list of attribute sets to grant when this ability set is applied.
  - **Type**: `TArray<FAttributeSetData>`
  - **Description**: Contains data for each attribute set to be added to the actor.

## Methods
- **GiveToAbilitySystem**: Grants the ability set to the specified ability system component.
  - **Parameters**:
    - `UBaseAbilitySystemComponent* BaseAsc`: The ability system component to which the abilities, effects, and attributes will be granted.
    - `FGrantedHandlesData* OutGrantedHandles`: A structure to store the handles of granted abilities, effects, and attributes.
    - `UObject* SourceObject`: The source object associated with the granted items (optional).
  - **Description**: Applies all gameplay abilities, effects, and attributes defined in the ability set to the specified ability system component.

- **GrantGameplayAbilities**: Grants gameplay abilities to the ability system component.
  - **Parameters**:
    - `UBaseAbilitySystemComponent* BaseAsc`: The ability system component to which the abilities will be granted.
    - `FGrantedHandlesData* OutGrantedHandles`: A structure to store the handles of granted abilities.
    - `UObject* SourceObject`: The source object associated with the granted abilities (optional).
  - **Description**: Iterates through the `GrantedGameplayAbilities` list and grants each ability to the specified ability system component.

- **GrantGameplayEffects**: Grants gameplay effects to the ability system component.
  - **Parameters**:
    - `UBaseAbilitySystemComponent* BaseAsc`: The ability system component to which the effects will be granted.
    - `FGrantedHandlesData* OutGrantedHandles`: A structure to store the handles of granted effects.
  - **Description**: Iterates through the `GrantedGameplayEffects` list and applies each effect to the specified ability system component.

- **GrantAttributeSets**: Grants attribute sets to the ability system component.
  - **Parameters**:
    - `UBaseAbilitySystemComponent* BaseAsc`: The ability system component to which the attributes will be granted.
    - `FGrantedHandlesData* OutGrantedHandles`: A structure to store the pointers to granted attribute sets.
  - **Description**: Iterates through the `GrantedAttributes` list and adds each attribute set to the specified ability system component.

## Events
- None.

## Overrides
- None.

## Usage
1. Define a `UBaseAbilitySet` in the editor and populate it with abilities, effects, and attributes.
2. Use the `GiveToAbilitySystem` method to apply the ability set to an actor's ability system component.
3. Store the granted handles in an `FGrantedHandlesData` structure for later removal if needed.

## Notes
- Ensure that the granted abilities, effects, and attributes are properly configured in the editor.
- Use this class to centralize and organize gameplay-related data for easy reuse.
- This class is typically used during character initialization or when applying temporary buffs.

## See Also
- [UBaseAbilitySystemComponent](UBaseAbilitySystemComponent.md)
- [FGameplayAbilityData](GameplayAbilityData.md)
- [FGameplayEffectData](GameplayEffectData.md)
