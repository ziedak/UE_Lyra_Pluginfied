# IBotControllerInterface

## Overview
`IBotControllerInterface` defines an interface for managing bot controllers. It provides methods for restarting bot controllers.

## Features
- Allows bot controllers to be restarted programmatically.

---

## Methods

### ServerRestartController
- **Description**: Restarts the bot controller on the server.

---

## Usage
1. Implement `IBotControllerInterface` in bot controller classes.
2. Use `ServerRestartController` to handle bot controller restarts.

## Notes
- This interface is designed to work with custom game modes and bot management systems.
- Ensure that bot controllers implement the required logic for restarting.

## See Also
- [ABaseGameMode](BaseGameMode.md)
- [AController](https://docs.unrealengine.com/)
