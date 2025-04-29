# ULyraPerformanceStatSubsystem

## Overview
`ULyraPerformanceStatSubsystem` is a `UGameInstanceSubsystem` that provides access to performance statistics for display purposes. It uses an observer (`FLyraPerformanceStatCache`) to cache performance data for the previous frame.

## Properties
- **Tracker**: A shared pointer to the performance stat cache.
  - **Type**: `TSharedPtr<FLyraPerformanceStatCache>`
  - **Description**: Tracks and caches performance data for display.

## Methods

### ULyraPerformanceStatSubsystem
- **GetCachedStat**: Retrieves the cached value of a specific performance stat.
  - **Parameters**:
    - `EDisplayablePerformanceStat Stat`: The performance stat to retrieve.
  - **Return Type**: `double`
  - **Description**: Returns the cached value of the specified performance stat.

- **Initialize**: Initializes the subsystem.
  - **Parameters**:
    - `FSubsystemCollectionBase& Collection`: The subsystem collection.
  - **Description**: Sets up the performance stat tracker.

- **Deinitialize**: Deinitializes the subsystem.
  - **Description**: Cleans up resources used by the tracker.

### FLyraPerformanceStatCache
- **StartCharting**: Starts tracking performance data.
- **ProcessFrame**: Processes performance data for the current frame.
  - **Parameters**:
    - `const FFrameData& FrameData`: The frame data to process.
- **StopCharting**: Stops tracking performance data.
- **GetCachedStat**: Retrieves the cached value of a specific performance stat.
  - **Parameters**:
    - `EDisplayablePerformanceStat Stat`: The performance stat to retrieve.
  - **Return Type**: `double`

## Usage
1. Use `ULyraPerformanceStatSubsystem` to access performance statistics in your game.
2. Call `GetCachedStat` to retrieve specific performance stats for display.
3. The subsystem automatically tracks and caches performance data for each frame.

## Notes
- This subsystem is designed to simplify the retrieval and display of performance statistics.
- It integrates with Unreal Engine's subsystem framework for easy access.

## See Also
- [UGameInstanceSubsystem](https://docs.unrealengine.com/)
- [IPerformanceDataConsumer](https://docs.unrealengine.com/)
