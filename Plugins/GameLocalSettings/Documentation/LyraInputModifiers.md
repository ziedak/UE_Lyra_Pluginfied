# LyraInputModifiers

## Overview
The `LyraInputModifiers` module contains various input modifiers used to customize input behavior based on game settings. These modifiers include scaling, dead zones, sensitivity adjustments, and axis inversion.

## Classes

### ULyraSettingBasedScalar
- **Description**: Scales input values based on scalar properties defined in shared user settings.
- **Properties**:
  - `XAxisScalarSettingName`: Name of the property for scaling the X-axis.
  - `YAxisScalarSettingName`: Name of the property for scaling the Y-axis.
  - `ZAxisScalarSettingName`: Name of the property for scaling the Z-axis.
  - `MaxValueClamp`: Maximum value for each axis.
  - `MinValueClamp`: Minimum value for each axis.
- **Overrides**:
  - `ModifyRaw_Implementation`: Modifies the raw input value based on the scalar settings.

### ULyraInputModifierDeadZone
- **Description**: Applies dead zones to input values based on settings in shared game settings.
- **Properties**:
  - `Type`: Type of dead zone (e.g., radial).
  - `UpperThreshold`: Threshold above which input is clamped to 1.
  - `DeadzoneStick`: Specifies whether the dead zone applies to the movement or look stick.
- **Overrides**:
  - `ModifyRaw_Implementation`: Modifies input values based on dead zone thresholds.
  - `GetVisualizationColor_Implementation`: Visualizes the dead zone effect.

### ULyraInputModifierGamepadSensitivity
- **Description**: Adjusts input sensitivity based on gamepad settings.
- **Properties**:
  - `TargetingType`: Type of targeting sensitivity (e.g., normal or ADS).
  - `SensitivityLevelTable`: Asset containing sensitivity scalar values.
- **Overrides**:
  - `ModifyRaw_Implementation`: Modifies input values based on sensitivity settings.

### ULyraInputModifierAimInversion
- **Description**: Inverts axis values based on settings in shared game settings.
- **Overrides**:
  - `ModifyRaw_Implementation`: Applies axis inversion to input values.

## Enums

### EDeadzoneStick
- **Description**: Specifies which stick the dead zone applies to.
- **Values**:
  - `MoveStick`: Dead zone for the movement stick.
  - `LookStick`: Dead zone for the looking stick.

### ELyraTargetingType
- **Description**: Specifies the type of targeting sensitivity.
- **Values**:
  - `Normal`: Sensitivity for normal look-around behavior.
  - `ADS`: Sensitivity for aiming down sights.

## Usage
1. Use the provided input modifiers to customize input behavior in your game.
2. Configure properties such as scalar settings, dead zone thresholds, and sensitivity levels in the editor or through code.
3. Integrate the modifiers into your input system to apply the desired effects.

## Notes
- These modifiers are designed to work seamlessly with the Lyra shared game settings.
- They provide a flexible way to adjust input behavior based on user preferences.

## See Also
- [UInputModifier](https://docs.unrealengine.com/)
