# UAsyncAction_ExperienceReady

## Overview
`UAsyncAction_ExperienceReady` is a blueprint async action that waits for the game state to be ready and valid, then triggers an event when the experience is ready.

## Features
- Asynchronously waits for the experience to be determined and loaded.
- Triggers a delegate when the experience is ready.
- Supports immediate callback if the experience is already valid.

---

## Properties

### OnExperienceReadyAsyncDelegate
- **Type**: `FExperienceReadyAsyncDelegate`
- **Description**: Delegate triggered when the experience is ready.

---

## Methods

### WaitForExperienceReady
- **Description**: Creates an async action to wait for the experience to be ready.
- **Parameters**:
  - `UObject* WorldContextObject`: The world context.
- **Returns**: `UAsyncAction_ExperienceReady*` - The async action instance.

### Activate
- **Description**: Activates the async action.

---

## Usage
1. Call `WaitForExperienceReady` in a blueprint to create the async action.
2. Bind to `OnExperienceReadyAsyncDelegate` to handle the experience-ready event.
3. The delegate will be triggered when the experience is determined and loaded.

## Notes
- This class is designed for use in blueprints to handle experience readiness asynchronously.
- Ensure that the game state and experience are properly configured.

## See Also
- [UExperienceDefinition_DA](ExperienceDefinition_DA.md)
- [UExperienceManagerComponent](ExperienceManagerComponent.md)
