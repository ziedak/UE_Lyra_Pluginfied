# CameraAssistInterface

## Overview
`ICameraAssistInterface` is an interface that provides methods for assisting with camera behavior. It is typically implemented by actors or controllers that need to influence camera behavior dynamically.

## Features
- **Camera Penetration Handling**: Provides methods for handling camera penetration scenarios.
- **Custom Camera Behavior**: Allows implementing classes to define custom behavior for camera interactions.

## Methods
- **GetCameraPreventPenetrationTarget()**: Returns an optional target actor to use for preventing camera penetration. This allows the implementing class to specify a custom target for collision checks.
- **OnCameraPenetratingTarget()**: Called when the camera is penetrating the target. This allows the implementing class to respond to penetration events, such as adjusting the camera or notifying the player.

## Usage
This interface is implemented by actors or controllers that need to influence camera behavior. For example, a player controller might implement this interface to provide a custom target for camera penetration checks or to handle penetration events.
