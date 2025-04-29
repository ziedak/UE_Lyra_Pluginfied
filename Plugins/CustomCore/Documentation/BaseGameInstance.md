# UBaseGameInstance

## Overview
`UBaseGameInstance` extends `UCommonGameInstance` to provide additional functionality for managing session joining, encryption, and lifecycle events in the game instance.

## Features
- Handles session joining requests.
- Manages network encryption tokens and acknowledgments.
- Provides hooks for initialization and shutdown logic.
- Supports pre-client travel customization.

---

## Properties

### DebugTestEncryptionKey
- **Type**: `TArray<uint8>`
- **Description**: A hard-coded encryption key used for testing encryption. **Note**: This is not secure and should not be used in production.

---

## Methods

### CanJoinRequestedSession
- **Description**: Determines if the requested session can be joined.
- **Returns**: `bool` - Whether the session can be joined.

### ReceivedNetworkEncryptionToken
- **Description**: Handles the receipt of a network encryption token.
- **Parameters**:
  - `const FString& EncryptionToken`: The encryption token.
  - `const FOnEncryptionKeyResponse& Delegate`: The delegate to call with the response.

### ReceivedNetworkEncryptionAck
- **Description**: Handles the receipt of a network encryption acknowledgment.
- **Parameters**:
  - `const FOnEncryptionKeyResponse& Delegate`: The delegate to call with the response.

### Init
- **Description**: Initializes the game instance. Called during the startup process.

### Shutdown
- **Description**: Shuts down the game instance. Called during the shutdown process.

### OnPreClientTravelToSession
- **Description**: Customizes the URL before the client travels to a session.
- **Parameters**:
  - `FString& URL`: The URL to modify.

---

## Usage
1. Use `UBaseGameInstance` as the base class for your game instance.
2. Override methods like `Init` and `Shutdown` to add custom initialization and cleanup logic.
3. Implement encryption handling using `ReceivedNetworkEncryptionToken` and `ReceivedNetworkEncryptionAck`.

## Notes
- This class is designed to integrate with Unreal Engine's session and encryption systems.
- Avoid using `DebugTestEncryptionKey` in production environments.

## See Also
- [UCommonGameInstance](https://docs.unrealengine.com/)
- [Session Management](https://docs.unrealengine.com/)
