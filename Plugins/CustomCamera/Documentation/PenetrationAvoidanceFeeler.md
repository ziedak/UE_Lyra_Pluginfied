# PenetrationAvoidanceFeeler

## Overview
`FPenetrationAvoidanceFeeler` is a struct that defines a feeler ray used for camera penetration avoidance. It helps ensure that the camera remains outside of objects by dynamically adjusting its position based on collisions.

## Features
- **Collision Detection**: Defines parameters for detecting collisions with the world and pawns.
- **Dynamic Adjustment**: Adjusts the camera's position based on the results of collision checks.
- **Customizable Behavior**: Allows for fine-tuning of collision behavior using weights, extents, and intervals.

## Properties
- **AdjustmentRot** (`FRotator`): Describes the deviation of the feeler ray from the main ray.
- **WorldWeight** (`float`): Determines how much the feeler affects the final position if it hits the world.
- **PawnWeight** (`float`): Determines how much the feeler affects the final position if it hits a pawn. Setting this to `0` disables collision with pawns.
- **Extent** (`float`): The extent used for collision when tracing this feeler.
- **TraceInterval** (`int32`): The minimum frame interval between traces with this feeler if nothing was hit in the last frame.
- **FramesUntilNextTrace** (`int32`): The number of frames remaining until this feeler can be used again.

## Constructors
- **Default Constructor**: Initializes all properties to default values.
- **Parameterized Constructor**: Allows for initializing all properties with custom values.

## Usage
This struct is used in conjunction with camera modes, such as `UCameraMode_ThirdPerson`, to dynamically adjust the camera's position and prevent it from penetrating objects in the game world.
