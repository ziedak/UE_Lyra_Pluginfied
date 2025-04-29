# FBaseAssetManagerStartupJob

## Overview
`FBaseAssetManagerStartupJob` is a structure that handles startup jobs for the asset manager. It provides functionality for managing job execution and progress reporting.

## Features
- Supports synchronous and asynchronous job execution.
- Reports progress through delegates.
- Tracks job metadata like name and weight.

---

## Properties

### JobName
- **Type**: `FString`
- **Description**: The name of the startup job.

### JobWeight
- **Type**: `float`
- **Description**: The weight of the job, used for progress calculation.

### SubstepProgressDelegate
- **Type**: `FBaseAssetManagerStartupJobSubstepProgress`
- **Description**: Delegate for reporting substep progress.

---

## Methods

### FBaseAssetManagerStartupJob
- **Description**: Constructor for initializing a startup job.
- **Parameters**:
  - `const FString& InJobName`: The name of the job.
  - `const TFunction<void(const FBaseAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)>& InJobFunc`: The job function.
  - `float InJobWeight`: The weight of the job.

### DoJob
- **Description**: Executes the job and returns a streamable handle if applicable.
- **Returns**: `TSharedPtr<FStreamableHandle>` - The handle for the job.

### UpdateSubstepProgress
- **Description**: Updates the progress of the job.
- **Parameters**:
  - `float NewProgress`: The new progress value.

---

## Usage
1. Create an instance of `FBaseAssetManagerStartupJob` with a job function and weight.
2. Call `DoJob` to execute the job.
3. Use `UpdateSubstepProgress` to report progress during execution.

## Notes
- This structure is used internally by the asset manager to manage startup tasks.
- Ensure that job functions are thread-safe if they involve asynchronous operations.

## See Also
- [UBaseAssetManager](BaseAssetManager.md)
- [FStreamableHandle](https://docs.unrealengine.com/)
