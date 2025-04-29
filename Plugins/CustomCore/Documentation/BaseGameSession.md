# ABaseGameSession

## Overview
`ABaseGameSession` extends `AGameSession` to provide custom behavior for handling auto-login and session management.

## Features
- Overrides the default auto-login behavior.
- Integrates with custom game mode logic for dedicated server login.

---

## Methods

### ProcessAutoLogin
- **Description**: Overrides the default behavior to disable auto-login processing.
- **Returns**: `bool` - Always returns `true` to indicate that auto-login is handled elsewhere.

---

## Usage
1. Use `ABaseGameSession` as the base class for your game session.
2. Override additional methods as needed to customize session behavior.

## Notes
- This class is designed to work with `ALyraGameMode` for dedicated server login handling.
- Ensure that session-related logic is implemented in the game mode or other relevant classes.

## See Also
- [AGameSession](https://docs.unrealengine.com/)
- [ALyraGameMode](BaseGameMode.md)
