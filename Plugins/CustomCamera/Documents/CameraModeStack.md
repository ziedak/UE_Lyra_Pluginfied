# CameraModeStack

## Overview
`CameraModeStack` is a class used to manage and blend multiple camera modes. It provides functionality for stacking and evaluating camera modes dynamically, ensuring smooth transitions and priority handling.

## Features
- **Dynamic Stack Management**: Maintains a stack of active camera modes, allowing for dynamic addition and removal.
- **Blending Support**: Blends between camera modes based on their weights and blend times, ensuring smooth transitions.
- **Activation and Deactivation**: Notifies camera modes when they are activated or deactivated, allowing for custom behavior.

## Properties
- **CameraModeInstances** (`TArray<UCustomCameraMode*>`): A list of all camera mode instances, used for reuse and memory management.
- **CameraModeStack** (`TArray<UCustomCameraMode*>`): The stack of active camera modes, ordered by priority.

## Methods
- **ActivateStack()**: Activates the stack, notifying all camera modes that they are active.
- **DeactivateStack()**: Deactivates the stack, notifying all camera modes that they are inactive.
- **PushCameraMode(const TSubclassOf<UCustomCameraMode>& CameraModeClass)**: Adds a new camera mode to the top of the stack, blending it in.
- **EvaluateStack(float DeltaTime, FCameraModeView& OutCameraModeView)**: Evaluates the stack and blends the active camera modes to produce the final view.
- **UpdateStack(float DeltaTime)**: Updates the stack, removing irrelevant modes and updating active ones.
- **BlendStack(FCameraModeView& OutCameraModeView)**: Blends the active camera modes in the stack to produce the final view.
- **DrawDebug(UCanvas* Canvas)**: Draws debug information about the stack and its camera modes.

## Usage
This class is used to manage the active camera modes and blend between them during gameplay. It is typically used in conjunction with `LyraCameraComponent` to provide a flexible and dynamic camera system.
