// Fill out your copyright notice in the Description page of Project Settings.
#include "Settings/SettingsTags.h"

namespace PlatformTags
{
	UE_DEFINE_GAMEPLAY_TAG(TRAIT_INPUT_SUPPORTS_GAMEPAD, "Platform.Trait.Input.SupportsGamepad");
	UE_DEFINE_GAMEPLAY_TAG(TRAIT_INPUT_SUPPORTS_TRIGGER_HAPTICS,
	                       "Platform.Trait.Input.SupportsTriggerHaptics");
	UE_DEFINE_GAMEPLAY_TAG(TRAIT_SUPPORTS_CHANGING_AUDIO_OUTPUT_DEVICE,
	                       "Platform.Trait.SupportsChangingAudioOutputDevice");
	UE_DEFINE_GAMEPLAY_TAG(TRAIT_SUPPORTS_BACKGROUND_AUDIO, "Platform.Trait.SupportsBackgroundAudio");

	UE_DEFINE_GAMEPLAY_TAG(TRAIT_SUPPORTS_WINDOWED_MODE, "Platform.Trait.SupportsWindowedMode");
	UE_DEFINE_GAMEPLAY_TAG(TRAIT_NEEDS_BRIGHTNESS_ADJUSTMENT, "Platform.Trait.NeedsBrightnessAdjustment");
}

namespace GameSettingsTags
{
	UE_DEFINE_GAMEPLAY_TAG(ACTION_EDIT_SAFEZONE, "GameSettings.Action.EditSafeZone");
	UE_DEFINE_GAMEPLAY_TAG(ACTION_EDIT_BRIGHTNESS, "GameSettings.Action.EditBrightness");
}