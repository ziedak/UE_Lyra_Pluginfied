# FGlobalAppliedAbilityList

## Overview
`FGlobalAppliedAbilityList` is a utility structure that manages a global list of gameplay abilities applied to ability system components. It provides methods to add, remove, and clear abilities across multiple components.

## Features
- Tracks applied abilities using a map of ability system components and their corresponding ability spec handles.
- Provides methods to add abilities to specific components, remove abilities from components, and clear all applied abilities.

---

## Properties

### Handles
- **Type**: `TMap<TObjectPtr<UBaseAbilitySystemComponent>, FGameplayAbilitySpecHandle>`
- **Description**: Maps ability system components to their corresponding gameplay ability spec handles.

---

## Methods

### AddToAsc
- **Description**: Adds a gameplay ability to the specified ability system component.
- **Parameters**:
  - `const TSubclassOf<UGameplayAbility>& Ability`: The gameplay ability to add.
  - `UBaseAbilitySystemComponent* Asc`: The ability system component to which the ability will be added.

### RemoveFromAsc
- **Description**: Removes a gameplay ability from the specified ability system component.
- **Parameters**:
  - `UBaseAbilitySystemComponent* ASC`: The ability system component from which the ability will be removed.

### RemoveFromAll
- **Description**: Removes all gameplay abilities from all tracked ability system components.

---

## Usage
1. Use `AddToAsc` to apply a gameplay ability to a specific ability system component.
2. Call `RemoveFromAsc` to remove a gameplay ability from a specific component.
3. Use `RemoveFromAll` to clear all applied abilities from all components.

## Notes
- Ensure that the ability system components are valid before adding or removing abilities.
- This structure is useful for managing global or shared abilities across multiple actors.

## See Also
- [UGameplayAbility](https://docs.unrealengine.com/)
- [UBaseAbilitySystemComponent](UBaseAbilitySystemComponent.md)
- [Gameplay Ability System](https://docs.unrealengine.com/)
