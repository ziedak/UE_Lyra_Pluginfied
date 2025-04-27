# LyraCameraComponent

## Overview
`LyraCameraComponent` is a custom camera component derived from `UCameraComponent`. It provides additional functionality for managing camera modes and blending, making it suitable for dynamic gameplay scenarios.

## Features
- **Dynamic Camera Mode Management**: Supports blending between multiple camera modes using a stack.
- **Field of View Adjustment**: Allows for runtime adjustment of the field of view.
- **Synchronization**: Keeps the player controller and camera component in sync with the latest view.
- **Debugging Tools**: Includes methods for drawing debug information on the screen.

## Properties
- **CameraModeStack** (`UCameraModeStack*`): A stack used to blend between different camera modes.
- **FieldOfViewOffset** (`float`): An offset applied to the field of view for temporary adjustments.

## Methods
- **OnRegister()**: Initializes the camera mode stack when the component is registered.
- **GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)**: Updates the camera view based on the active camera modes and blends.
- **UpdateCameraModes()**: Updates the stack of camera modes based on the current gameplay state.
- **DrawDebug(UCanvas* Canvas)**: Draws debug information about the camera modes and their states.

## Usage
This component is used to manage and blend camera modes dynamically during gameplay. It is typically attached to a player character or pawn to provide a flexible and responsive camera system.
