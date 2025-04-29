# UBaseGameplayCueManager

## Overview
`UBaseGameplayCueManager` is a custom implementation of the `UGameplayCueManager` class. It provides additional functionality for managing gameplay cues, including delayed loading, preloading, and handling always-loaded cues.

This class must be declared in the `DefaultGame.ini` file as follows:
```
[/Script/GameplayAbilities.AbilitySystemGlobals]
GlobalGameplayCueManagerClass=/Script/<GameName>.BaseGameplayCueManager
```

## Features
- Supports delayed loading of gameplay cues.
- Handles always-loaded cues for critical gameplay functionality.
- Provides methods for debugging and refreshing gameplay cue assets.

---

## Properties

### PreloadedCues
- **Type**: `TSet<TObjectPtr<UClass>>`
- **Description**: Tracks gameplay cues preloaded on the client due to content references.

### AlwaysLoadedCues
- **Type**: `TSet<TObjectPtr<UClass>>`
- **Description**: Tracks gameplay cues that are always loaded (e.g., code-referenced or explicitly marked).

### PreloadedCueReferencers
- **Type**: `TMap<FObjectKey, TSet<FObjectKey>>`
- **Description**: Maps preloaded cues to their referencers.

### LoadedGameplayTagsToProcess
- **Type**: `TArray<FLoadedGameplayTagToProcessData>`
- **Description**: Stores gameplay tags that need to be processed after loading.

---

## Methods

### Get
- **Description**: Retrieves the singleton instance of the `UBaseGameplayCueManager`.

### OnCreated
- **Description**: Called when the gameplay cue manager is created. Initializes the manager.

### ShouldAsyncLoadRuntimeObjectLibraries
- **Description**: Determines whether runtime object libraries should be loaded asynchronously.
- **Returns**: `bool`

### ShouldSyncLoadMissingGameplayCues
- **Description**: Determines whether missing gameplay cues should be loaded synchronously.
- **Returns**: `bool`

### ShouldAsyncLoadMissingGameplayCues
- **Description**: Determines whether missing gameplay cues should be loaded asynchronously.
- **Returns**: `bool`

### DumpGameplayCues
- **Description**: Outputs a list of all gameplay cues for debugging purposes.
- **Parameters**:
  - `const TArray<FString>& Args`: Arguments for the dump command.

### LoadAlwaysLoadedCues
- **Description**: Loads gameplay cues that are marked as always loaded.

### RefreshGameplayCuePrimaryAsset
- **Description**: Updates the bundles for the primary gameplay cue asset.

### OnGameplayTagLoaded
- **Description**: Handles the event when a gameplay tag is loaded.
- **Parameters**:
  - `const FGameplayTag& Tag`: The loaded gameplay tag.

### HandlePostGarbageCollect
- **Description**: Processes gameplay tags after garbage collection.

### ProcessLoadedTags
- **Description**: Processes gameplay tags that were loaded.

### ProcessTagToPreload
- **Description**: Preloads a gameplay cue associated with a tag.
- **Parameters**:
  - `const FGameplayTag& Tag`: The gameplay tag to preload.
  - `UObject* OwningObject`: The object owning the tag.

### OnPreloadCueComplete
- **Description**: Called when a gameplay cue preload is complete.
- **Parameters**:
  - `FSoftObjectPath Path`: The path of the preloaded cue.
  - `TWeakObjectPtr<UObject> OwningObject`: The owning object.
  - `bool bAlwaysLoadedCue`: Whether the cue is always loaded.

### RegisterPreloadedCue
- **Description**: Registers a preloaded gameplay cue.
- **Parameters**:
  - `UClass* LoadedGameplayCueClass`: The class of the loaded gameplay cue.
  - `UObject* OwningObject`: The owning object.

### HandlePostLoadMap
- **Description**: Handles events after a map is loaded.
- **Parameters**:
  - `UWorld* NewWorld`: The newly loaded world.

### UpdateDelayLoadDelegateListeners
- **Description**: Updates listeners for delayed loading of gameplay cues.

### ShouldDelayLoadGameplayCues
- **Description**: Determines whether gameplay cues should be delayed.
- **Returns**: `bool`

---

## Usage
1. Declare `UBaseGameplayCueManager` in the `DefaultGame.ini` file as the global gameplay cue manager.
2. Use the `LoadAlwaysLoadedCues` method to ensure critical gameplay cues are preloaded.
3. Call `DumpGameplayCues` for debugging purposes to list all registered gameplay cues.

## Notes
- This class is designed to enhance the default `UGameplayCueManager` with additional functionality for managing gameplay cues efficiently.
- Ensure that always-loaded cues are properly configured to avoid runtime issues.

## See Also
- [UGameplayCueManager](https://docs.unrealengine.com/)
- [Gameplay Ability System](https://docs.unrealengine.com/)
