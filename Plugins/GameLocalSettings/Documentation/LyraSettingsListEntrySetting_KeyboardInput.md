# ULyraSettingsListEntrySetting_KeyboardInput

## Overview
`ULyraSettingsListEntrySetting_KeyboardInput` is a subclass of `UGameSettingListEntry_Setting` that provides functionality for managing keyboard input settings in the Lyra game framework. It allows users to bind keys, reset bindings, and handle duplicate key warnings.

## Properties

### Transient Properties
- **OriginalKeyToBind**: Stores the original key to bind.
  - **Type**: `FKey`
  - **Default Value**: `EKeys::Invalid`

### Protected Properties
- **KeyboardInputSetting**: The keyboard input setting associated with this entry.
  - **Type**: `TObjectPtr<ULyraSettingKeyboardInput>`

- **PressAnyKeyPanelClass**: The class used for the "Press Any Key" panel.
  - **Type**: `TSubclassOf<UGameSettingPressAnyKey>`

- **KeyAlreadyBoundWarningPanelClass**: The class used for the "Key Already Bound" warning panel.
  - **Type**: `TSubclassOf<UKeyAlreadyBoundWarning>`

### Private Bound Widgets
- **Button_PrimaryKey**: The button for binding the primary key.
  - **Type**: `TObjectPtr<UCommonButtonBase>`

- **Button_SecondaryKey**: The button for binding the secondary key.
  - **Type**: `TObjectPtr<UCommonButtonBase>`

- **Button_Clear**: The button for clearing the key binding.
  - **Type**: `TObjectPtr<UCommonButtonBase>`

- **Button_ResetToDefault**: The button for resetting the key binding to default.
  - **Type**: `TObjectPtr<UCommonButtonBase>`

## Methods

### Public Methods
- **SetSetting**: Sets the associated game setting.
  - **Parameters**:
    - `UGameSetting* InSetting`: The game setting to associate.

### Protected Methods
- **NativeOnInitialized**: Called when the widget is initialized.
- **NativeOnEntryReleased**: Called when the entry is released.
- **OnSettingChanged**: Called when the associated setting changes.

- **HandlePrimaryKeyClicked**: Handles the primary key button click.
- **HandleSecondaryKeyClicked**: Handles the secondary key button click.
- **HandleClearClicked**: Handles the clear button click.
- **HandleResetToDefaultClicked**: Handles the reset-to-default button click.

- **HandlePrimaryKeySelected**: Handles the selection of a primary key.
  - **Parameters**:
    - `const FKey& InKey`: The selected key.
    - `UGameSettingPressAnyKey* PressAnyKeyPanel`: The "Press Any Key" panel.

- **HandleSecondaryKeySelected**: Handles the selection of a secondary key.
  - **Parameters**:
    - `const FKey& InKey`: The selected key.
    - `UGameSettingPressAnyKey* PressAnyKeyPanel`: The "Press Any Key" panel.

- **HandlePrimaryDuplicateKeySelected**: Handles the selection of a duplicate primary key.
  - **Parameters**:
    - `const FKey& InKey`: The duplicate key.
    - `UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel`: The warning panel.

- **HandleSecondaryDuplicateKeySelected**: Handles the selection of a duplicate secondary key.
  - **Parameters**:
    - `const FKey& InKey`: The duplicate key.
    - `UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel`: The warning panel.

- **ChangeBinding**: Changes the key binding.
  - **Parameters**:
    - `int32 InKeyBindSlot`: The key bind slot (primary or secondary).
    - `const FKey& InKey`: The new key.

- **HandleKeySelectionCanceled**: Handles the cancellation of key selection.
  - **Overloads**:
    - `UGameSettingPressAnyKey* PressAnyKeyPanel`
    - `UKeyAlreadyBoundWarning* PressAnyKeyPanel`

- **Refresh**: Refreshes the widget.

## Usage
1. Use `ULyraSettingsListEntrySetting_KeyboardInput` to manage keyboard input settings in your game.
2. Bind keys using the primary and secondary key buttons.
3. Handle duplicate key warnings using the provided panels.
4. Reset bindings to default or clear them as needed.

## Notes
- This class is designed to integrate with the Lyra game framework's settings system.
- It provides a user-friendly interface for managing keyboard input bindings.

## See Also
- [UGameSettingListEntry_Setting](https://docs.unrealengine.com/)
- [ULyraSettingKeyboardInput](https://docs.unrealengine.com/)
- [UGameSettingPressAnyKey](https://docs.unrealengine.com/)
