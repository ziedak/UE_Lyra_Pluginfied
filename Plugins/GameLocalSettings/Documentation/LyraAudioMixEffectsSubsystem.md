# ULyraAudioMixEffectsSubsystem

## Overview
`ULyraAudioMixEffectsSubsystem` is a world subsystem that manages audio control bus mixes and submix effect chains. It applies user-defined audio settings and dynamic range effects based on the configuration in `ULyraAudioSettings`.

## Properties
- **DefaultBaseMix**: The default sound control bus mix.
  - **Type**: `TObjectPtr<USoundControlBusMix>`
  - **Description**: Retrieved from `ULyraAudioSettings` and applied as the base mix.

- **LoadingScreenMix**: The sound control bus mix for loading screens.
  - **Type**: `TObjectPtr<USoundControlBusMix>`
  - **Description**: Applied during loading screens to manage background audio.

- **UserMix**: The user-defined sound control bus mix.
  - **Type**: `TObjectPtr<USoundControlBusMix>`
  - **Description**: Applies user-specific audio settings.

- **HDRSubmixEffectChain**: Submix effect chains for HDR audio.
  - **Type**: `TArray<FLyraAudioSubmixEffectsChain>`
  - **Description**: Overrides submix effects when HDR audio is enabled.

- **LDRSubmixEffectChain**: Submix effect chains for LDR audio.
  - **Type**: `TArray<FLyraAudioSubmixEffectsChain>`
  - **Description**: Overrides submix effects when HDR audio is disabled.

## Methods
- **ApplyDynamicRangeEffectsChains**: Applies HDR or LDR submix effect chains.
  - **Parameters**:
    - `bool bHDRAudio`: Whether to apply HDR audio effects.
  - **Description**: Updates the submix effect chains based on the user's dynamic range preference.

- **OnLoadingScreenStatusChanged**: Handles loading screen status changes.
  - **Parameters**:
    - `bool bShowingLoadingScreen`: Whether the loading screen is currently visible.
  - **Description**: Applies or removes the loading screen mix based on the status.

- **LoadSubmixEffectChain**: Loads submix effect chains from a configuration map.
  - **Parameters**:
    - `const TArray<FLyraSubmixEffectChainMap>& SubmixEffectChainMap`: The configuration map.
  - **Return Type**: `TArray<FLyraAudioSubmixEffectsChain>`
  - **Description**: Converts configuration data into runtime submix effect chains.

## Usage
1. Configure `ULyraAudioSettings` with the desired control bus mixes and submix effect chains.
2. The subsystem automatically applies these settings during gameplay.
3. Use `ApplyDynamicRangeEffectsChains` to toggle between HDR and LDR audio dynamically.

## Notes
- This subsystem is automatically initialized and managed by the engine.
- Ensure that the control buses and submixes referenced in `ULyraAudioSettings` are valid.

## See Also
- [ULyraAudioSettings](LyraAudioSettings.md)
- [USoundControlBusMix](https://docs.unrealengine.com/)
- [USoundEffectSubmixPreset](https://docs.unrealengine.com/)
