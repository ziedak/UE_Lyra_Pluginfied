# IPlayerSpawnInterface

## Overview
`IPlayerSpawnInterface` defines an interface for managing player spawn locations. It provides methods for claiming spawn points and checking their occupancy status.

## Features
- Allows controllers to claim spawn locations.
- Provides occupancy status for spawn locations.

---

## Methods

### TryClaim
- **Description**: Attempts to claim a spawn location for the specified player.
- **Parameters**:
  - `AController* Player`: The player controller attempting to claim the location.
- **Returns**: `bool` - Whether the claim was successful.

### GetLocationOccupancy
- **Description**: Gets the occupancy status of a spawn location.
- **Parameters**:
  - `AController* Controller`: The player controller associated with the location.
- **Returns**: `EPlayerStartLocationOccupancy` - The occupancy status of the location.

---

## Usage
1. Implement `IPlayerSpawnInterface` in classes that manage player spawn locations.
2. Use `TryClaim` to handle spawn point allocation.
3. Use `GetLocationOccupancy` to check the status of spawn locations.

## Notes
- This interface is designed to work with `UPlayerSpawningManagerComponent` for managing player spawns.
- Ensure that spawn locations are properly configured and managed.

## See Also
- [UPlayerSpawningManagerComponent](PlayerSpawningManagerComponent.md)
- [APlayerStart](https://docs.unrealengine.com/)
