# UICameraManagerComponent

## Overview
`UICameraManagerComponent` is a component class derived from `UActorComponent`. It is designed to manage UI-specific camera behavior in the context of the `ALyraPlayerCameraManager`.

## Features
- Manages the view target for UI interactions.
- Provides functionality to update the camera view when UI elements take priority over gameplay.

## Properties
- **ViewTarget** (`AActor*`): The actor that the camera is currently targeting.
- **bUpdatingViewTarget** (`bool`): Indicates whether the view target is being updated.

## Usage
This component is typically used within the `ALyraPlayerCameraManager` to handle camera transitions and behaviors specific to UI interactions.
