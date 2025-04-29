# ULyraAudioSettings

## Overview
`ULyraAudioSettings` is a developer settings class that provides configuration for audio control bus mixes and submix effect chains. It allows customization of audio behavior, including dynamic range settings and volume control.

## Properties
- **DefaultControlBusMix**: The default base control bus mix.
  - **Type**: `FSoftObjectPath`
  - **Description**: Specifies the default control bus mix used for audio settings.

- **LoadingScreenControlBusMix**: The control bus mix used during loading screens.
  - **Type**: `FSoftObjectPath`
  - **Description**: Covers background audio events during loading screens.

- **UserSettingsControlBusMix**: The control bus mix for user settings.
  - **Type**: `FSoftObjectPath`
  - **Description**: Applies user-defined audio settings.

- **OverallVolumeControlBus**: Control bus for overall sound volume.
  - **Type**: `FSoftObjectPath`
  - **Description**: Adjusts the master volume for all audio.

- **MusicVolumeControlBus**: Control bus for music volume.
  - **Type**: `FSoftObjectPath`
  - **Description**: Adjusts the volume of music tracks.

- **SoundFXVolumeControlBus**: Control bus for sound effects volume.
  - **Type**: `FSoftObjectPath`
  - **Description**: Adjusts the volume of sound effects.

- **DialogueVolumeControlBus**: Control bus for dialogue volume.
  - **Type**: `FSoftObjectPath`
  - **Description**: Adjusts the volume of dialogue audio.

- **VoiceChatVolumeControlBus**: Control bus for voice chat volume.
  - **Type**: `FSoftObjectPath`
  - **Description**: Adjusts the volume of voice chat.

- **HDRAudioSubmixEffectChain**: Submix effect chains for high dynamic range audio.
  - **Type**: `TArray<FLyraSubmixEffectChainMap>`
  - **Description**: Defines submix processing chains for HDR audio output.

- **LDRAudioSubmixEffectChain**: Submix effect chains for low dynamic range audio.
  - **Type**: `TArray<FLyraSubmixEffectChainMap>`
  - **Description**: Defines submix processing chains for LDR audio output.

## Usage
1. Configure the control bus paths and submix effect chains in the editor.
2. Use these settings in conjunction with audio subsystems to apply dynamic range effects and volume adjustments.

## Notes
- This class is designed to centralize audio configuration for the project.
- Ensure that the specified control buses and submixes exist in the project.

## See Also
- [USoundControlBus](https://docs.unrealengine.com/)
- [USoundEffectSubmixPreset](https://docs.unrealengine.com/)
