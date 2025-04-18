// Copyright Epic Games, Inc. All Rights Reserved.

#include "RegistrySettings/LyraGameSettingRegistry.h"
#include "CustomSettings/LyraSettingValueDiscreteDynamic_AudioOutputDevice.h"
// #include "DataSource/GameSettingDataSource.h"
#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "GameSettingCollection.h"
#include "GameSettingValueScalarDynamic.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
#include "Settings/SettingsTags.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"
// #include "Settings/MyClass.h"

class ULocalPlayer;

#define LOCTEXT_NAMESPACE "Lyra"


UGameSettingCollection* ULyraGameSettingRegistry::InitializeAudioSettings(ULocalPlayer* InLocalPlayer)
{
	const auto Screen = UGameSettingCollection::CreateCollection(TEXT("AudioCollection"),
	                                                             LOCTEXT("AudioCollection_Name", "Audio"));
	Screen->Initialize(InLocalPlayer);
	Screen->AddSetting(AddVolumeSettings());
	Screen->AddSetting(AddSoundSettings());
	return Screen;
}


UGameSettingCollection* ULyraGameSettingRegistry::AddVolumeSettings()
{
	const auto Volume = UGameSettingCollection::CreateCollection(TEXT("VolumeCollection"),
	                                                             LOCTEXT("VolumeCollection_Name", "Volume"));

	Volume->AddSetting(AddVolume(TEXT("OverallVolume"), LOCTEXT("OverallVolume_Name", "Overall"),
	                             LOCTEXT("OverallVolume_Description", "Adjusts the volume of everything."),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(GetOverallVolume),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(SetOverallVolume),
	                             GetDefault<ULyraSettingsLocal>()->GetOverallVolume()));
	Volume->AddSetting(AddVolume(TEXT("MusicVolume"), LOCTEXT("MusicVolume_Name", "Music"),
	                             LOCTEXT("MusicVolume_Description", "Adjusts the volume of music."),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(GetMusicVolume),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(SetMusicVolume),
	                             GetDefault<ULyraSettingsLocal>()->GetMusicVolume()));
	Volume->AddSetting(AddVolume(TEXT("SoundEffectsVolume"), LOCTEXT("SoundEffectsVolume_Name", "Sound Effects"),
	                             LOCTEXT("SoundEffectsVolume_Description",
	                                     "Adjusts the volume of sound effects."),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(GetSoundFXVolume),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(SetSoundFXVolume),
	                             GetDefault<ULyraSettingsLocal>()->GetSoundFXVolume()));
	Volume->AddSetting(AddVolume(TEXT("DialogueVolume"), LOCTEXT("DialogueVolume_Name", "Dialogue"),
	                             LOCTEXT("DialogueVolume_Description",
	                                     "Adjusts the volume of dialogue for game characters and voice overs."),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDialogueVolume),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDialogueVolume),
	                             GetDefault<ULyraSettingsLocal>()->GetDialogueVolume()));
	Volume->AddSetting(AddVolume(TEXT("VoiceChatVolume"), LOCTEXT("VoiceChatVolume_Name", "Voice Chat"),
	                             LOCTEXT("VoiceChatVolume_Description", "Adjusts the volume of voice chat."),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(GetVoiceChatVolume),
	                             GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVoiceChatVolume),
	                             GetDefault<ULyraSettingsLocal>()->GetVoiceChatVolume()));
	return Volume;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::AddVolume(const FName& DevName,
                                                                    const FText& DisplayName,
                                                                    const FText& Description,
                                                                    const TSharedRef<FGameSettingDataSource>&
                                                                    Getter,
                                                                    const TSharedRef<FGameSettingDataSource>&
                                                                    Setter,
                                                                    const float DefaultValue)
{
	return UGameSettingValueScalarDynamic::CreateSettings(DevName,
	                                                      DisplayName,
	                                                      Description,
	                                                      Getter,
	                                                      Setter,
	                                                      DefaultValue,
	                                                      UGameSettingValueScalarDynamic::ZeroToOnePercent,
	                                                      FWhenPlayingAsPrimaryPlayer::Get());
}

UGameSettingCollection* ULyraGameSettingRegistry::AddSoundSettings()
{
	auto* Sound = UGameSettingCollection::CreateCollection("SoundCollection", LOCTEXT("SoundCollection_Name", "Sound"));
	Sound->AddSetting(AddSubtitleSettings());
	Sound->AddSetting(AddAudioOutputDeviceSetting());
	Sound->AddSetting(AddBackgroundAudioSetting());
	Sound->AddSetting(AddHeadphoneModeSetting());
	Sound->AddSetting(AddHDRAudioModeSetting());
	return Sound;
}

UGameSettingCollectionPage* ULyraGameSettingRegistry::AddSubtitleSettings()
{
	const auto SubtitlePage = UGameSettingCollectionPage::CreateSettings(
		TEXT("SubtitlePage"),
		LOCTEXT("SubtitlePage_Name", "Subtitles"),
		LOCTEXT("SubtitlePage_Description", "Configure the visual appearance of subtitles."),
		LOCTEXT("SubtitlePage_Navigation", "Options"),
		FWhenPlayingAsPrimaryPlayer::Get());


	const auto SubtitleCollection = UGameSettingCollection::CreateCollection(
		"SubtitlesCollection", LOCTEXT("SubtitlesCollection_Name", "Subtitles"));
	SubtitlePage->AddSetting(SubtitleCollection);
	

	const auto Subtitles = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		TEXT("Subtitles"), LOCTEXT("Subtitles_Name", "Subtitles"),
		LOCTEXT("Subtitles_Description", "Turns subtitles on/off."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesEnabled),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesEnabled),
		GetDefault<ULyraSettingsShared>()->GetSubtitlesEnabled());
	SubtitleCollection->AddSetting(Subtitles);
	// SubtitleTextSize
	const auto SubtitleTextSize = UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings(
		"SubtitleTextSize", LOCTEXT("SubtitleTextSize_Name", "Text Size"),
		LOCTEXT("SubtitleTextSize_Description", "Choose different sizes of the subtitle text."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextSize),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextSize),
		GetDefault<ULyraSettingsShared>()->GetSubtitlesTextSize(),
		{
			{
				ESubtitleDisplayTextSize::ExtraSmall, LOCTEXT("ESubtitleTextSize_ExtraSmall", "Extra Small")
			},
			{ESubtitleDisplayTextSize::Small, LOCTEXT("ESubtitleTextSize_Small", "Small")},
			{ESubtitleDisplayTextSize::Medium, LOCTEXT("ESubtitleTextSize_Medium", "Medium")},
			{ESubtitleDisplayTextSize::Large, LOCTEXT("ESubtitleTextSize_Large", "Large")},
			{
				ESubtitleDisplayTextSize::ExtraLarge, LOCTEXT("ESubtitleTextSize_ExtraLarge", "Extra Large")
			}
		}
	);
	SubtitleCollection->AddSetting(SubtitleTextSize);
	// SubtitleTextColor
	const auto SubtitleTextColor = UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings(

		TEXT("SubtitleTextColor"), LOCTEXT("SubtitleTextColor_Name", "Text Color"),
		LOCTEXT("SubtitleTextColor_Description", "Choose different colors for the subtitle text."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextColor),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextColor),
		GetDefault<ULyraSettingsShared>()->GetSubtitlesTextColor(), {
			{ESubtitleDisplayTextColor::White, LOCTEXT("ESubtitleTextColor_White", "White")},
			{ESubtitleDisplayTextColor::Black, LOCTEXT("ESubtitleTextColor_Black", "Black")},
			{ESubtitleDisplayTextColor::Red, LOCTEXT("ESubtitleTextColor_Red", "Red")},
			{ESubtitleDisplayTextColor::Green, LOCTEXT("ESubtitleTextColor_Green", "Green")},
			{ESubtitleDisplayTextColor::Blue, LOCTEXT("ESubtitleTextColor_Blue", "Blue")},
			{ESubtitleDisplayTextColor::Yellow, LOCTEXT("ESubtitleTextColor_Yellow", "Yellow")},
			{ESubtitleDisplayTextColor::Cyan, LOCTEXT("ESubtitleTextColor_Cyan", "Cyan")},
			{ESubtitleDisplayTextColor::Magenta, LOCTEXT("ESubtitleTextColor_Magenta", "Magenta")}
		});
	SubtitleCollection->AddSetting(SubtitleTextColor);
	// SubtitleTextBorder
	const auto SubtitleTextBorder = UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings(

		TEXT("SubtitleTextBorder"),
		LOCTEXT("SubtitleBackgroundStyle_Name", "Text Border"),
		LOCTEXT("SubtitleTextBorder_Description", "Choose different borders for the text."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextBorder),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextBorder),
		GetDefault<ULyraSettingsShared>()->GetSubtitlesTextBorder(), {
			{ESubtitleDisplayTextBorder::None, LOCTEXT("ESubtitleTextBorder_None", "None")},
			{ESubtitleDisplayTextBorder::Outline, LOCTEXT("ESubtitleTextBorder_Outline", "Outline")},
			{
				ESubtitleDisplayTextBorder::DropShadow,
				LOCTEXT("ESubtitleTextBorder_DropShadow", "Drop Shadow")
			}
		});
	SubtitleCollection->AddSetting(SubtitleTextBorder);
	// SubtitleBackgroundStyle
	const auto SubtitleBackgroundOpacity = UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings(
		TEXT("SubtitleBackgroundOpacity"),
		LOCTEXT("SubtitleBackground_Name", "Background Opacity"),
		LOCTEXT("SubtitleBackgroundOpacity_Description",
		        "Choose a different background or letterboxing for the subtitles."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesBackgroundOpacity),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesBackgroundOpacity),
		GetDefault<ULyraSettingsShared>()->GetSubtitlesBackgroundOpacity(), {
			{
				ESubtitleDisplayBackgroundOpacity::Clear,
				LOCTEXT("ESubtitleBackgroundOpacity_Clear", "Clear")
			},
			{
				ESubtitleDisplayBackgroundOpacity::Low,
				LOCTEXT("ESubtitleBackgroundOpacity_Low", "Low")
			},
			{
				ESubtitleDisplayBackgroundOpacity::Medium,
				LOCTEXT("ESubtitleBackgroundOpacity_Medium", "Medium")
			},
			{
				ESubtitleDisplayBackgroundOpacity::High,
				LOCTEXT("ESubtitleBackgroundOpacity_High", "High")
			},
			{
				ESubtitleDisplayBackgroundOpacity::Solid,
				LOCTEXT("ESubtitleBackgroundOpacity_Solid", "Solid")
			}
		});
	SubtitleCollection->AddSetting(SubtitleBackgroundOpacity);

	return SubtitlePage;
}

ULyraSettingValueDiscreteDynamic_AudioOutputDevice* ULyraGameSettingRegistry::AddAudioOutputDeviceSetting()
{
	const auto Setting = NewObject<
		ULyraSettingValueDiscreteDynamic_AudioOutputDevice>();
	Setting->SetDevName(TEXT("AudioOutputDevice"));
	Setting->SetDisplayName(LOCTEXT("AudioOutputDevice_Name", "Audio Output Device"));
	Setting->SetDescriptionRichText(LOCTEXT("AudioOutputDevice_Description",
	                                        "Changes the audio output device for game audio (not voice chat)."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetAudioOutputDeviceId));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetAudioOutputDeviceId));
	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(PlatformTags::TRAIT_SUPPORTS_CHANGING_AUDIO_OUTPUT_DEVICE,
	                                                               TEXT(
		                                                               "Platform does not support changing audio output device")));
	return Setting;
}

UGameSettingValueDiscreteDynamic_Enum* ULyraGameSettingRegistry::AddBackgroundAudioSetting()
{
	const auto Setting = UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings(
		TEXT("BackgroundAudio"), LOCTEXT("BackgroundAudio_Name", "Background Audio"),
		LOCTEXT("BackgroundAudio_Description",
		        "Turns game audio on/off when the game is in the background. When on, the game audio will continue to play when the game is minimized, or another window is focused."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetAllowAudioInBackgroundSetting),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetAllowAudioInBackgroundSetting),
		GetDefault<ULyraSettingsShared>()->GetAllowAudioInBackgroundSetting(), {
			{EBackgroundAudioSetting::Off, LOCTEXT("EBackgroundAudioSetting_Off", "Off")},
			{EBackgroundAudioSetting::AllSounds, LOCTEXT("EBackgroundAudioSetting_AllSounds", "All Sounds")}
		});
	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(PlatformTags::TRAIT_SUPPORTS_BACKGROUND_AUDIO,
	                                                               TEXT("Platform does not support background audio")));
	return Setting;
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::AddHeadphoneModeSetting()
{
	const auto Setting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		TEXT("HeadphoneMode"), LOCTEXT("HeadphoneMode_Name", "3D Headphones"),
		LOCTEXT("HeadphoneMode_Description",
		        "Enable binaural audio. Provides 3D audio spatialization, so you can hear the location of sounds more precisely, including above, below, and behind you. Recommended for use with stereo headphones only."),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(bDesiredHeadphoneMode),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(bDesiredHeadphoneMode),
		GetDefault<ULyraSettingsLocal>()->IsHeadphoneModeEnabled());
	Setting->AddEditCondition(MakeShared<FWhenCondition>(
		[](const ULocalPlayer*, FGameSettingEditableState& InOutEditState){
			if (!GetDefault<ULyraSettingsLocal>()->CanModifyHeadphoneModeEnabled()) InOutEditState.Kill(TEXT("Binaural Spatialization option cannot be modified on this platform"));
		}));
	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	return Setting;
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::AddHDRAudioModeSetting()
{
	const auto Setting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		TEXT("HDRAudioMode"), LOCTEXT("HDRAudioMode_Name", "High Dynamic Range Audio"),
		LOCTEXT("HDRAudioMode_Description",
		        "Enable high dynamic range audio. Changes the runtime processing chain to increase the dynamic range of the audio mix-down, appropriate for theater or more cinematic experiences."),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(bUseHDRAudioMode),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(SetHDRAudioModeEnabled),
		GetDefault<ULyraSettingsLocal>()->IsHDRAudioModeEnabled());
	Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	return Setting;
}
#undef LOCTEXT_NAMESPACE