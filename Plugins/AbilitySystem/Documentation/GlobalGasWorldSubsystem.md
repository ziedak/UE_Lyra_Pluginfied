# UGlobalGasWorldSubsystem

## Overview
`UGlobalGasWorldSubsystem` is a world subsystem that provides a centralized system for managing globally-applied gameplay abilities and effects. It allows for the application, removal, and management of global effects and interactions across all registered ability system components (ASCs) in the game world.

## Features
- Apply and remove gameplay abilities globally to all registered ASCs.
- Apply and remove gameplay effects globally to all registered ASCs.
- Automatically register and unregister ASCs to manage global effects and abilities.
- Supports various global gameplay scenarios, such as area-of-effect damage, environmental effects, and global buffs/debuffs.

---

## Properties

### AppliedAbilities
- **Type**: `TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList>`
- **Description**: Tracks globally-applied gameplay abilities and their associated ASCs.

### AppliedEffects
- **Type**: `TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList>`
- **Description**: Tracks globally-applied gameplay effects and their associated ASCs.

### RegisteredASCs
- **Type**: `TArray<TObjectPtr<UBaseAbilitySystemComponent>>`
- **Description**: Stores all registered ability system components in the game world.

---

## Methods

### ApplyAbilityToAll
- **Description**: Applies a gameplay ability to all registered ASCs.
- **Parameters**:
  - `TSubclassOf<UGameplayAbility> Ability`: The gameplay ability to apply.

### ApplyEffectToAll
- **Description**: Applies a gameplay effect to all registered ASCs.
- **Parameters**:
  - `TSubclassOf<UGameplayEffect> Effect`: The gameplay effect to apply.

### RemoveAbilityFromAll
- **Description**: Removes a gameplay ability from all registered ASCs.
- **Parameters**:
  - `TSubclassOf<UGameplayAbility> Ability`: The gameplay ability to remove.

### RemoveEffectFromAll
- **Description**: Removes a gameplay effect from all registered ASCs.
- **Parameters**:
  - `TSubclassOf<UGameplayEffect> Effect`: The gameplay effect to remove.

### RegisterAsc
- **Description**: Registers an ASC with the global system and applies any active global effects or abilities.
- **Parameters**:
  - `UBaseAbilitySystemComponent* Asc`: The ability system component to register.

### UnregisterAsc
- **Description**: Unregisters an ASC from the global system and removes any active global effects or abilities.
- **Parameters**:
  - `UBaseAbilitySystemComponent* Asc`: The ability system component to unregister.

---

## Usage
1. Use `ApplyAbilityToAll` or `ApplyEffectToAll` to apply global abilities or effects to all registered ASCs.
2. Call `RemoveAbilityFromAll` or `RemoveEffectFromAll` to remove global abilities or effects from all registered ASCs.
3. Register ASCs using `RegisterAsc` to include them in the global system.
4. Unregister ASCs using `UnregisterAsc` to exclude them from the global system.

## Notes
- Ensure that ASCs are properly registered and unregistered to maintain consistency in global effects and abilities.
- This subsystem is ideal for managing global gameplay mechanics, such as environmental effects, global buffs, or world events.

## See Also
- [UGameplayAbility](https://docs.unrealengine.com/)
- [UGameplayEffect](https://docs.unrealengine.com/)
- [UBaseAbilitySystemComponent](UBaseAbilitySystemComponent.md)
- [Gameplay Ability System](https://docs.unrealengine.com/)
