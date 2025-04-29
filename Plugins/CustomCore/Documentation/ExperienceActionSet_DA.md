# UExperienceActionSet_DA

## Overview
`UExperienceActionSet_DA` is a data asset that defines a set of actions to perform as part of entering a gameplay experience. It supports modular composition of actions and game feature dependencies.

## Features
- Defines actions to execute during experience activation and deactivation.
- Specifies game feature plugins required for the action set.

---

## Properties

### GameFeatureActions
- **Type**: `TArray<TObjectPtr<UGameFeatureAction>>`
- **Description**: List of actions to perform during the experience lifecycle.

### GameFeaturesToEnableList
- **Type**: `TArray<FString>`
- **Description**: List of game feature plugins required for this action set.

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
1. Create a `UExperienceActionSet_DA` asset to define a set of actions.
2. Add `GameFeatureActions` to specify the actions to execute.
3. Configure `GameFeaturesToEnableList` to define required game feature plugins.

## Notes
- This asset is designed to be used with `UExperienceDefinition_DA` for modular experience composition.
- Ensure that all referenced actions and plugins are valid and properly configured.

## See Also
- [UExperienceDefinition_DA](ExperienceDefinition_DA.md)
- [UGameFeatureAction](GameFeatureAction.md)
