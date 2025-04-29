# UBaseAssetManager

## Overview
`UBaseAssetManager` is a custom implementation of the Unreal Engine Asset Manager. It provides game-specific logic for managing assets and startup jobs.

## Features
- Manages game-specific asset loading and tracking.
- Provides utility methods for synchronous asset and class loading.
- Handles startup jobs for initializing game content.
- Tracks loaded assets to keep them in memory.

---

## Properties

### StartupJobs
- **Type**: `TArray<FBaseAssetManagerStartupJob>`
- **Description**: The list of tasks to execute on startup.

### LoadedAssets
- **Type**: `TSet<TObjectPtr<const UObject>>`
- **Description**: Tracks assets loaded and kept in memory.

---

## Methods

### Get
- **Description**: Returns the singleton instance of the asset manager.
- **Returns**: `UBaseAssetManager&` - The asset manager instance.

### GetAsset
- **Description**: Synchronously loads an asset if not already loaded.
- **Parameters**:
  - `const TSoftObjectPtr<AssetType>& AssetPointer`: The asset pointer.
  - `bool bKeepInMemory`: Whether to keep the asset in memory.
- **Returns**: `AssetType*` - The loaded asset.

### GetSubclass
- **Description**: Synchronously loads a subclass if not already loaded.
- **Parameters**:
  - `const TSoftClassPtr<AssetType>& AssetPointer`: The class pointer.
  - `bool bKeepInMemory`: Whether to keep the class in memory.
- **Returns**: `TSubclassOf<AssetType>` - The loaded subclass.

### DoAllStartupJobs
- **Description**: Executes all startup jobs in the `StartupJobs` array.

---

## Usage
1. Use `UBaseAssetManager` as the asset manager class in your project settings.
2. Add startup jobs to the `StartupJobs` array and call `DoAllStartupJobs` to process them.
3. Use `GetAsset` and `GetSubclass` for synchronous asset and class loading.

## Notes
- This class is designed to integrate with the Gameplay Ability System and other game-specific systems.
- Ensure that all assets referenced by `TSoftObjectPtr` or `TSoftClassPtr` are valid and properly configured.

## See Also
- [FBaseAssetManagerStartupJob](BaseAssetManagerStartupJob.md)
- [UAssetManager](https://docs.unrealengine.com/)
