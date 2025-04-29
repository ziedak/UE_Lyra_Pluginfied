# ULyraSettingKeyboardInput

## Overview
`ULyraSettingKeyboardInput` is a class derived from `UGameSettingValue`. It provides functionality for managing and customizing keyboard input settings, including key bindings and their associated actions.

## Properties
- **ActionMappingName**: The name of the action's mappings.
  - **Type**: `FName`
  - **Description**: Identifies the action associated with this setting.

- **QueryOptions**: Options to filter keys for this setting.
  - **Type**: `FPlayerMappableKeyQueryOptions`
  - **Description**: Specifies the criteria for querying mappable keys.

- **ProfileIdentifier**: The profile identifier for this key setting.
  - **Type**: `FGameplayTag`
  - **Description**: Identifies the key profile associated with this setting.

- **InitialKeyMappings**: Stores the initial key mappings for each slot.
  - **Type**: `TMap<EPlayerMappableKeySlot, FKey>`
  - **Description**: Tracks the original key bindings for restoration purposes.

## Methods
- **InitializeInputData**: Initializes input data for the key profile and mapping.
  - **Parameters**:
    - `const UEnhancedPlayerMappableKeyProfile* KeyProfile`: The key profile to initialize.
    - `const FKeyMappingRow& MappingData`: The mapping data to use.
    - `const FPlayerMappableKeyQueryOptions& InQueryOptions`: The query options for filtering keys.

- **GetKeyTextFromSlot**: Retrieves the key text for a specific slot.
  - **Parameters**:
    - `const EPlayerMappableKeySlot InSlot`: The slot to retrieve the key text from.
  - **Return Type**: `FText`

- **ChangeBinding**: Changes the key binding for a specific slot.
  - **Parameters**:
    - `int32 InKeyBindSlot`: The slot to change.
    - `const FKey& NewKey`: The new key to bind.
  - **Return Type**: `bool`

- **GetAllMappedActionsFromKey**: Retrieves all actions mapped to a specific key.
  - **Parameters**:
    - `int32 InKeyBindSlot`: The slot to check.
    - `const FKey& Key`: The key to query.
    - `TArray<FName>& OutActionNames`: The output array of action names.

- **IsMappingCustomized**: Checks if the mappings have been customized.
  - **Return Type**: `bool`

- **GetSettingDisplayName**: Retrieves the display name of the setting.
  - **Return Type**: `FText`

- **GetSettingDisplayCategory**: Retrieves the display category of the setting.
  - **Return Type**: `FText`

- **StoreInitial**: Stores the initial state of the setting.
- **ResetToDefault**: Resets the setting to its default state.
- **RestoreToInitial**: Restores the setting to its initial state.

- **FindKeyMappingRow**: Finds the key mapping row associated with this setting.
  - **Return Type**: `const FKeyMappingRow*`

- **FindMappableKeyProfile**: Finds the mappable key profile associated with this setting.
  - **Return Type**: `UEnhancedPlayerMappableKeyProfile*`

- **GetUserSettings**: Retrieves the user settings.
  - **Return Type**: `UEnhancedInputUserSettings*`

## Overrides
- **UGameSettingValue**:
  - `StoreInitial`
  - `ResetToDefault`
  - `RestoreToInitial`

- **ULyraSetting**:
  - `OnInitialized`

## Usage
1. Create an instance of `ULyraSettingKeyboardInput`.
2. Use `InitializeInputData` to set up the key profile and mapping data.
3. Customize key bindings using `ChangeBinding`.
4. Retrieve or display key information using methods like `GetKeyTextFromSlot` or `GetSettingDisplayName`.

## Notes
- This class is designed to simplify the management of keyboard input settings.
- Deprecated methods like `GetPrimaryKeyText` and `GetSecondaryKeyText` should be replaced with `GetKeyTextFromSlot`.

## See Also
- [UGameSettingValue](https://docs.unrealengine.com/)
- [Enhanced Input System](https://docs.unrealengine.com/)
