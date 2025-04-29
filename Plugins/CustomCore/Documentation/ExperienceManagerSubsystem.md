# UExperienceManagerSubsystem

## Overview
`UExperienceManagerSubsystem` is an engine subsystem that manages experiences, primarily for arbitration between multiple PIE (Play In Editor) sessions.

## Features
- Tracks and manages game feature plugin activation requests.
- Provides functionality for plugin activation and deactivation in PIE sessions.
- Supports editor-specific behavior for managing plugins.

---

## Properties

### GameFeaturePluginRequestCountMap
- **Type**: `TMap<FString, int32>`
- **Description**: Tracks the active request count for each game feature plugin.

---

## Methods

### OnPlayInEditorBegun
- **Description**: Handles logic when a PIE session begins.
- **Editor Only**: Available only in editor builds.

### NotifyOfPluginActivation
- **Description**: Notifies the subsystem of a plugin activation request.
- **Parameters**:
  - `const FString& PluginURL`: The URL of the plugin to activate.

### RequestToDeactivatePlugin
- **Description**: Requests the deactivation of a plugin.
- **Parameters**:
  - `const FString& PluginURL`: The URL of the plugin to deactivate.
- **Returns**: `bool` - Whether the deactivation request was successful.

---

## Usage
1. Use `NotifyOfPluginActivation` to register plugin activation requests.
2. Use `RequestToDeactivatePlugin` to manage plugin deactivation during PIE sessions.
3. Call `OnPlayInEditorBegun` to handle initialization logic when PIE begins.

## Notes
- This subsystem is primarily designed for managing plugins in editor builds.
- Ensure that plugin URLs are valid and properly configured.

## See Also
- [UExperienceManagerComponent](ExperienceManagerComponent.md)
- [UGameFeatureAction](GameFeatureAction.md)
