# CameraMode_ThirdPerson

## Overview
`CameraMode_ThirdPerson` is a specific implementation of `CustomCameraMode` designed for third-person camera behavior.

## Features
- Supports target offset curves for dynamic camera positioning.
- Provides collision handling to prevent camera penetration.
- Includes predictive avoidance for smoother camera transitions.

## Properties
- **TargetOffsetCurve** (`UCurveVector*`): A curve defining local-space offsets from the target based on view pitch.
- **bUseRuntimeFloatCurves** (`bool`): Indicates whether runtime float curves are used for target offsets.
- **TargetOffsetX/Y/Z** (`FRuntimeFloatCurve`): Runtime float curves for target offsets in X, Y, and Z axes.
- **CrouchOffsetBlendMultiplier** (`float`): Multiplier for blending crouch offsets.
- **PenetrationBlendInTime** (`float`): Time to blend in when avoiding penetration.
- **PenetrationBlendOutTime** (`float`): Time to blend out when avoiding penetration.
- **bPreventPenetration** (`bool`): If true, prevents the camera from penetrating the world.
- **bDoPredictiveAvoidance** (`bool`): If true, enables predictive avoidance for smoother transitions.
- **CollisionPushOutDistance** (`float`): Distance to push the camera out during collisions.
- **ReportPenetrationPercent** (`float`): Percentage of penetration to trigger reporting.
- **PenetrationAvoidanceFeelers** (`TArray<FPenetrationAvoidanceFeeler>`): Feelers used for collision detection.
- **DebugActorsHitDuringCameraPenetration** (`TArray<AActor*>`): Debug list of actors hit during camera penetration.

## Methods
- **UpdateView(float DeltaTime)**: Updates the camera view based on the elapsed time.
- **UpdateForTarget(float DeltaTime)**: Updates the camera for the current target actor.
- **UpdatePreventPenetration(float DeltaTime)**: Adjusts the camera position to prevent penetration.
- **SetTargetCrouchOffset(FVector Offset)**: Sets the crouch offset for the target.
- **DrawDebug(UCanvas* Canvas)**: Draws debug information for the camera mode.

## Usage
This class is used to implement third-person camera behavior with advanced collision handling and dynamic positioning.
