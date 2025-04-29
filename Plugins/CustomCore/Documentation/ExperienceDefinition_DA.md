# UExperienceDefinition_DA

## Overview
`UExperienceDefinition_DA` is a data asset that defines a gameplay experience. It includes settings for game features, default pawn data, and actions to perform during the experience lifecycle.

## Features
- Specifies game feature plugins to activate.
- Defines default pawn data for players.
- Includes actions to execute during experience activation and deactivation.
- Supports additional action sets for modular composition.

---

## Properties

### GameFeaturesToEnableList
- **Type**: `TArray<FString>`
- **Description**: List of game feature plugins to activate for this experience.

### DefaultPawnData
- **Type**: `TObjectPtr<const UPrimaryDataAsset>`
- **Description**: The default pawn data to use for players.

### GameFeatureActions
- **Type**: `TArray<TObjectPtr<UGameFeatureAction>>`
- **Description**: List of actions to perform during the experience lifecycle.

### ExperienceActionSets
- **Type**: `TArray<TObjectPtr<UExperienceActionSet_DA>>`
- **Description**: Additional action sets to compose into this experience.

---

## Methods

### IsDataValid
- **Description**: Validates the data asset in the editor.
- **Parameters**:
  - `FDataValidationContext& Context`: The validation context.
- **Returns**: `EDataValidationResult` - The validation result.

### UpdateAssetBundleData
- **Description**: Updates the asset bundle data in the editor.

---

## Usage
1. Create a `UExperienceDefinition_DA` asset to define a gameplay experience.
2. Configure properties like `GameFeaturesToEnableList` and `DefaultPawnData`.
3. Add `GameFeatureActions` and `ExperienceActionSets` to define the experience's behavior.

## Notes
- This asset is designed to integrate with the `UExperienceManagerComponent` for experience management.
- Ensure that all referenced assets and plugins are valid and properly configured.

## See Also
- [UExperienceActionSet_DA](ExperienceActionSet_DA.md)
- [UGameFeatureAction](GameFeatureAction.md)
