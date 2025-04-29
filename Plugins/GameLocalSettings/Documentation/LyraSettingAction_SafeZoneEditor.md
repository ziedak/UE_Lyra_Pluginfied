# ULyraSettingAction_SafeZoneEditor

## Overview
`ULyraSettingAction_SafeZoneEditor` is a class derived from `UGameSettingAction`. It provides functionality for managing safe zone settings in the game.

## Properties
- **SafeZoneValueSetting**: A scalar dynamic setting for the safe zone value.
  - **Type**: `TObjectPtr<ULyraSettingValueScalarDynamic_SafeZoneValue>`
  - **Description**: Represents the safe zone value setting.

## Methods
- **ULyraSettingAction_SafeZoneEditor**: Constructor for the class.
  - **Description**: Initializes the safe zone editor setting.

- **GetChildSettings**: Retrieves the child settings of the safe zone editor.
  - **Return Type**: `TArray<UGameSetting*>`
  - **Description**: Returns the child settings, including the safe zone value setting.

## Nested Classes
- **ULyraSettingValueScalarDynamic_SafeZoneValue**: A nested class derived from `UGameSettingValueScalarDynamic`.
  - **Methods**:
    - **ResetToDefault**: Resets the safe zone value to its default state.
    - **RestoreToInitial**: Restores the safe zone value to its initial state.

## Events
- None.

## Overrides
- **UGameSettingAction**:
  - `GetChildSettings`

- **UGameSettingValueScalarDynamic** (for `ULyraSettingValueScalarDynamic_SafeZoneValue`):
  - `ResetToDefault`
  - `RestoreToInitial`

## Usage
1. Use `ULyraSettingAction_SafeZoneEditor` to manage safe zone settings in your game.
2. Access the `SafeZoneValueSetting` property to retrieve or modify the safe zone value.
3. Use the `GetChildSettings` method to retrieve the child settings.

## Notes
- This class is designed to simplify the management of safe zone settings in games.
- The nested `ULyraSettingValueScalarDynamic_SafeZoneValue` class provides additional functionality for scalar dynamic settings.

## See Also
- [UGameSettingAction](https://docs.unrealengine.com/)
- [UGameSettingValueScalarDynamic](https://docs.unrealengine.com/)
