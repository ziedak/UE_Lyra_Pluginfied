# ULyraGameSettingRegistry

## Overview
`ULyraGameSettingRegistry` is a class derived from `UGameSettingRegistry`. It provides a centralized registry for managing game settings, including video, audio, gameplay, and input settings.

## Properties
- **VideoSettings**: Collection of video-related settings.
  - **Type**: `UGameSettingCollection*`
  - **Description**: Contains settings for resolution, graphics quality, and frame rates.

- **AudioSettings**: Collection of audio-related settings.
  - **Type**: `UGameSettingCollection*`
  - **Description**: Contains settings for volume, audio output devices, and subtitles.

- **GameplaySettings**: Collection of gameplay-related settings.
  - **Type**: `UGameSettingCollection*`
  - **Description**: Contains settings for language, replays, and gameplay preferences.

- **MouseAndKeyboardSettings**: Collection of mouse and keyboard-related settings.
  - **Type**: `UGameSettingCollection*`
  - **Description**: Contains settings for sensitivity, key bindings, and axis inversion.

- **GamepadSettings**: Collection of gamepad-related settings.
  - **Type**: `UGameSettingCollection*`
  - **Description**: Contains settings for sensitivity, dead zones, and vibration.

## Methods

### Initialization
- **Get**: Retrieves the game setting registry for a specific local player.
  - **Parameters**:
    - `ULocalPlayer* InLocalPlayer`: The local player for which to retrieve the registry.
  - **Return Type**: `ULyraGameSettingRegistry*`

- **OnInitialize**: Initializes the registry for a specific local player.
  - **Parameters**:
    - `ULocalPlayer* InLocalPlayer`: The local player for which to initialize the registry.

### Saving and Validation
- **SaveChanges**: Saves changes made to the settings.
- **IsFinishedInitializing**: Checks if the registry has finished initializing.
  - **Return Type**: `bool`

- **IsOwningLocalPlayerValid**: Checks if the owning local player is valid.
  - **Return Type**: `bool`

- **AreSharedSettingsAvailable**: Checks if shared settings are available.
  - **Return Type**: `bool`

### Video Settings
- **InitializeVideoSettings**: Initializes video-related settings.
  - **Parameters**:
    - `ULocalPlayer* InLocalPlayer`: The local player for which to initialize video settings.
  - **Return Type**: `UGameSettingCollection*`

- **AddDisplaySettings**: Adds display-related settings to the video settings collection.
- **AddGraphicsSettings**: Adds graphics-related settings to the video settings collection.
- **AddAdvancedGraphicsSettings**: Adds advanced graphics settings to the video settings collection.

### Audio Settings
- **InitializeAudioSettings**: Initializes audio-related settings.
  - **Parameters**:
    - `ULocalPlayer* InLocalPlayer`: The local player for which to initialize audio settings.
  - **Return Type**: `UGameSettingCollection*`

- **AddSoundSettings**: Adds sound-related settings to the audio settings collection.
- **AddSubtitleSettings**: Adds subtitle-related settings to the audio settings collection.
- **AddVolumeSettings**: Adds volume-related settings to the audio settings collection.

### Input Settings
- **InitializeMouseAndKeyboardSettings**: Initializes mouse and keyboard-related settings.
  - **Parameters**:
    - `ULocalPlayer* InLocalPlayer`: The local player for which to initialize input settings.
  - **Return Type**: `UGameSettingCollection*`

- **InitializeGamepadSettings**: Initializes gamepad-related settings.
  - **Parameters**:
    - `ULocalPlayer* InLocalPlayer`: The local player for which to initialize gamepad settings.
  - **Return Type**: `UGameSettingCollection*`

### Gameplay Settings
- **InitializeGameplaySettings**: Initializes gameplay-related settings.
  - **Parameters**:
    - `ULocalPlayer* InLocalPlayer`: The local player for which to initialize gameplay settings.
  - **Return Type**: `UGameSettingCollection*`

## Usage
1. Use `ULyraGameSettingRegistry` to manage game settings for your project.
2. Call `InitializeVideoSettings`, `InitializeAudioSettings`, and other initialization methods to set up specific categories of settings.
3. Use `SaveChanges` to persist changes made to the settings.

## Notes
- This class is designed to centralize the management of game settings for easier access and modification.
- It integrates with Unreal Engine's settings framework for seamless configuration.

## See Also
- [UGameSettingRegistry](https://docs.unrealengine.com/)
- [UGameSettingCollection](https://docs.unrealengine.com/)
