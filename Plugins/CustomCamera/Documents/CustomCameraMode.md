# CustomCameraMode

## Overview
`CustomCameraMode` is an abstract base class for defining custom camera modes. It provides the foundation for implementing specific camera behaviors, such as third-person, first-person, or cinematic cameras.

## Features
- **Blending Support**: Supports blending between camera modes with customizable blend times and functions.
- **Field of View and Pitch Control**: Allows for defining field of view and view pitch limits to constrain the camera's movement.
- **Pivot-Based Camera Control**: Provides methods to calculate the pivot location and rotation for the camera.

## Properties
- **CameraTypeTag** (`FGameplayTag`): A tag used to identify the type of camera mode, useful for querying active modes.
- **FieldOfView** (`float`): The horizontal field of view in degrees, with customizable limits.
- **ViewPitchMin** (`float`): The minimum view pitch in degrees, used to constrain the camera's vertical movement.
- **ViewPitchMax** (`float`): The maximum view pitch in degrees, used to constrain the camera's vertical movement.
- **BlendTime** (`float`): The time it takes to blend into this mode, allowing for smooth transitions.
- **BlendFunction** (`ECameraModeBlendFunction`): The function used for blending, such as linear or ease-in/out.
- **BlendExponent** (`float`): The exponent used to control the shape of the blend curve.
- **bResetInterpolation** (`bool`): If true, skips interpolation and places the camera in the ideal location immediately.

## Methods
- **OnActivation()**: Called when the camera mode is activated, allowing for initialization or setup.
- **OnDeactivation()**: Called when the camera mode is deactivated, allowing for cleanup.
- **UpdateCamera(float DeltaTime)**: Updates the camera's state based on the elapsed time, such as adjusting position or rotation.
- **GetBlendWeight()**: Returns the current blend weight of the camera mode, used for blending calculations.
- **SetBlendWeight(float Weight)**: Sets the blend weight for the camera mode, influencing its contribution to the final view.
- **GetBlendTime()**: Returns the blend time for the camera mode, used for blending transitions.
- **GetPivotLocation()**: Returns the pivot location for the camera, typically based on the target actor's position.
- **GetPivotRotation()**: Returns the pivot rotation for the camera, typically based on the target actor's orientation.

## Usage
This class is extended to create specific camera modes with unique behaviors and settings. For example, a third-person camera mode might adjust its position based on the player's movement, while a cinematic camera mode might follow a predefined path.
