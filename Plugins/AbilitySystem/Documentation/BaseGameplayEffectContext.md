# FBaseGameplayEffectContext

## Overview
`FBaseGameplayEffectContext` is a custom implementation of `FGameplayEffectContext` that extends its functionality to include additional data, such as target data and ability source information. It is used to store and manage transient information during gameplay effect execution.

## Features
- Stores target data for gameplay effects.
- Tracks the ability source object and its level.
- Provides utility methods for accessing physical materials and duplicating the context.
- Supports network serialization for multiplayer gameplay.

---

## Properties

### TargetData
- **Type**: `FGameplayAbilityTargetDataHandle`
- **Description**: Stores target data associated with the gameplay effect.

### AbilitySourceObject
- **Type**: `TWeakObjectPtr<const UObject>`
- **Description**: Tracks the object used as the ability source. This is not replicated.

---

## Methods

### GetTargetData
- **Description**: Retrieves the target data associated with the gameplay effect.
- **Returns**: `FGameplayAbilityTargetDataHandle`

### AddTargetData
- **Description**: Appends target data to the existing target data handle.
- **Parameters**:
  - `const FGameplayAbilityTargetDataHandle& TargetDataHandle`: The target data to append.

### ExtractEffectContext
- **Description**: Extracts the `FBaseGameplayEffectContext` from a `FGameplayEffectContextHandle`.
- **Parameters**:
  - `FGameplayEffectContextHandle Handle`: The handle containing the effect context.
- **Returns**: `FBaseGameplayEffectContext*`

### SetAbilitySource
- **Description**: Sets the object used as the ability source and its level.
- **Parameters**:
  - `const IAbilitySourceInterface* InObject`: The ability source object.
  - `float InSourceLevel`: The level of the ability source.

### GetAbilitySource
- **Description**: Retrieves the ability source interface associated with the source object.
- **Returns**: `const IAbilitySourceInterface*`

### GetPhysicalMaterial
- **Description**: Retrieves the physical material from the hit result, if available.
- **Returns**: `const UPhysicalMaterial*`

---

## Overrides

### GetScriptStruct
- **Description**: Returns the script struct associated with this context.
- **Returns**: `UScriptStruct*`

### Duplicate
- **Description**: Creates a duplicate of the current effect context.
- **Returns**: `FBaseGameplayEffectContext*`

### NetSerialize
- **Description**: Serializes the effect context for network replication.
- **Parameters**:
  - `FArchive& Ar`: The archive to serialize to/from.
  - `UPackageMap* Map`: The package map for network serialization.
  - `bool& bOutSuccess`: Indicates whether serialization was successful.
- **Returns**: `bool`

---

## Usage
1. Use `FBaseGameplayEffectContext` to store additional data during gameplay effect execution.
2. Call `SetAbilitySource` to associate an ability source with the context.
3. Use `GetTargetData` and `AddTargetData` to manage target data for the effect.
4. Override `NetSerialize` to ensure proper replication in multiplayer scenarios.

## Notes
- This structure is designed to be extended for game-specific requirements.
- Ensure that the ability source object implements the `IAbilitySourceInterface` for proper functionality.

## See Also
- [FGameplayEffectContext](https://docs.unrealengine.com/)
- [Gameplay Ability System](https://docs.unrealengine.com/)
