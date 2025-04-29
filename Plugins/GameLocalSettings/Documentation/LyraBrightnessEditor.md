# ULyraBrightnessEditor

## Overview
`ULyraBrightnessEditor` is a class derived from `UCommonActivatableWidget` and implements the `IGameSettingActionInterface`. It provides functionality for editing brightness settings in the game.

## Properties
- **OnSafeZoneSet**: A multicast delegate triggered when the safe zone is set.
  - **Type**: `FSimpleMulticastDelegate`
  - **Description**: Used to notify listeners when the safe zone is configured.

- **bCanCancel**: Determines if the brightness editor can be canceled.
  - **Type**: `bool`
  - **Default**: `true`
  - **Description**: Specifies whether the user can cancel the brightness editor.

- **ValueSetting**: A weak pointer to the scalar value setting being edited.
  - **Type**: `TWeakObjectPtr<UGameSettingValueScalar>`
  - **Description**: Represents the brightness value setting.

- **Switcher_SafeZoneMessage**: A widget switcher for displaying safe zone messages.
  - **Type**: `TObjectPtr<UWidgetSwitcher>`
  - **Description**: Used to toggle between different safe zone messages.

- **RichText_Default**: A rich text block for displaying default messages.
  - **Type**: `TObjectPtr<UCommonRichTextBlock>`
  - **Description**: Displays default instructional text.

- **Button_Back**: A button for navigating back.
  - **Type**: `TObjectPtr<UCommonButtonBase>`
  - **Description**: Allows the user to cancel and go back.

- **Button_Done**: A button for confirming changes.
  - **Type**: `TObjectPtr<UCommonButtonBase>`
  - **Description**: Allows the user to confirm and save changes.

## Methods

### Public Methods
- **ExecuteActionForSetting_Implementation**: Executes an action for a specific game setting.
  - **Parameters**:
    - `FGameplayTag ActionTag`: The action tag to execute.
    - `UGameSetting* InSetting`: The game setting to act upon.
  - **Return Type**: `bool`
  - **Description**: Implements the `IGameSettingActionInterface` to handle actions for settings.

### Protected Methods
- **NativeOnActivated**: Called when the widget is activated.
  - **Description**: Handles logic for when the brightness editor is activated.

- **NativeOnInitialized**: Called when the widget is initialized.
  - **Description**: Handles initialization logic for the brightness editor.

- **NativeOnAnalogValueChanged**: Handles analog input changes.
  - **Parameters**:
    - `const FGeometry& InGeometry`: The geometry of the widget.
    - `const FAnalogInputEvent& InAnalogEvent`: The analog input event.
  - **Return Type**: `FReply`
  - **Description**: Processes analog input changes.

- **NativeOnMouseWheel**: Handles mouse wheel input.
  - **Parameters**:
    - `const FGeometry& InGeometry`: The geometry of the widget.
    - `const FPointerEvent& InMouseEvent`: The mouse wheel event.
  - **Return Type**: `FReply`
  - **Description**: Processes mouse wheel input.

### Private Methods
- **HandleInputModeChanged**: Handles changes to the input mode.
  - **Parameters**:
    - `ECommonInputType InInputType`: The new input type.
  - **Description**: Updates the widget based on the current input mode.

- **HandleBackClicked**: Handles the back button click event.
  - **Description**: Cancels the brightness editor and navigates back.

- **HandleDoneClicked**: Handles the done button click event.
  - **Description**: Confirms changes and saves the brightness setting.

## Usage
1. Use `ULyraBrightnessEditor` to provide a UI for editing brightness settings in your game.
2. Bind the `OnSafeZoneSet` delegate to handle safe zone configuration events.
3. Customize the `ValueSetting` property to link the brightness setting being edited.

## Notes
- This class is designed to simplify the process of editing brightness settings in a user-friendly way.
- It integrates with Unreal Engine's common UI framework for consistent behavior.

## See Also
- [UCommonActivatableWidget](https://docs.unrealengine.com/)
- [IGameSettingActionInterface](https://docs.unrealengine.com/)
