# UBaseActivatableWidget

## Overview
`UBaseActivatableWidget` is an abstract class derived from `UCommonActivatableWidget`. It provides functionality for managing input configurations and mouse behavior when the widget is activated.

## Properties
- **InputConfig**: The desired input mode to use while this UI is activated.
  - **Type**: `EWidgetInputMode`
  - **Description**: Determines whether key presses should still reach the game/player controller or be restricted to the UI.

- **GameMouseCaptureMode**: The desired mouse behavior when the game gets input.
  - **Type**: `EMouseCaptureMode`
  - **Description**: Specifies how the mouse is captured when interacting with the game.

## Methods
- **GetDesiredInputConfig**: Retrieves the desired input configuration for the widget.
  - **Return Type**: `TOptional<FUIInputConfig>`
  - **Description**: Called to determine the input configuration when the widget is activated.

- **ValidateCompiledWidgetTree** (Editor-only): Validates the widget tree during compilation.
  - **Parameters**:
    - `const UWidgetTree& BlueprintWidgetTree`: The widget tree to validate.
    - `IWidgetCompilerLog& CompileLog`: The log to report validation issues.
  - **Description**: Ensures the widget tree adheres to expected standards during compilation.

## Events
- None.

## Overrides
- **UCommonActivatableWidget**:
  - `GetDesiredInputConfig`
  - `ValidateCompiledWidgetTree` (Editor-only)

## Usage
1. Derive a new widget class from `UBaseActivatableWidget`.
2. Set the `InputConfig` and `GameMouseCaptureMode` properties in the derived class or in the editor.
3. Implement custom behavior for input configuration or mouse capture if needed.

## Notes
- This class is designed to simplify input and mouse behavior management for activatable widgets.
- It is abstract and must be subclassed to be used.

## See Also
- [UCommonActivatableWidget](https://docs.unrealengine.com/)
