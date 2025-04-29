# LyraPlayerCameraManager

## Overview
`LyraPlayerCameraManager` is a custom player camera manager class derived from `APlayerCameraManager`. It serves as the base camera manager for this project.

## Features
- Integrates with `UICameraManagerComponent` to handle UI-specific camera behavior.
- Provides a foundation for managing player camera transitions and settings.

## Properties
- **UICameraManager** (`UICameraManagerComponent*`): A reference to the UI camera manager component that controls camera behavior during UI interactions.

## Methods
- **OnUpdateCamera(float DeltaTime)**: Updates the camera state based on the elapsed time.
- **SetUICameraActive(bool bActive)**: Activates or deactivates the UI camera.
- **GetUICameraManager()**: Returns the `UICameraManagerComponent` instance.

## Usage
This class is used as the primary camera manager for players, allowing for seamless integration of gameplay and UI camera transitions.
