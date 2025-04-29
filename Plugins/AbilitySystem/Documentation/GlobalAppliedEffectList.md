# FGlobalAppliedEffectList

## Overview
`FGlobalAppliedEffectList` is a utility structure that manages a global list of gameplay effects applied to ability system components. It provides methods to add, remove, and clear effects across multiple components.

## Features
- Tracks applied gameplay effects using a map of ability system components and their corresponding active gameplay effect handles.
- Provides methods to add effects to specific components, remove effects from components, and clear all applied effects.

---

## Properties

### Handles
- **Type**: `TMap<TObjectPtr<UBaseAbilitySystemComponent>, FActiveGameplayEffectHandle>`
- **Description**: Maps ability system components to their corresponding active gameplay effect handles.

---

## Methods

### AddToAsc
- **Description**: Adds a gameplay effect to the specified ability system component.
- **Parameters**:
  - `const TSubclassOf<UGameplayEffect>& Effect`: The gameplay effect to add.
  - `UBaseAbilitySystemComponent* Asc`: The ability system component to which the effect will be added.

### RemoveFromAsc
- **Description**: Removes a gameplay effect from the specified ability system component.
- **Parameters**:
  - `UBaseAbilitySystemComponent* Asc`: The ability system component from which the effect will be removed.

### RemoveFromAll
- **Description**: Removes all gameplay effects from all tracked ability system components.

---

## Usage
1. Use `AddToAsc` to apply a gameplay effect to a specific ability system component.
2. Call `RemoveFromAsc` to remove a gameplay effect from a specific component.
3. Use `RemoveFromAll` to clear all applied effects from all components.

## Notes
- Ensure that the ability system components are valid before adding or removing effects.
- This structure is useful for managing global or shared effects across multiple actors.

## See Also
- [UGameplayEffect](https://docs.unrealengine.com/)
- [UBaseAbilitySystemComponent](UBaseAbilitySystemComponent.md)
- [Gameplay Ability System](https://docs.unrealengine.com/)
