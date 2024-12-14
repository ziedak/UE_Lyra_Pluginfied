// Copyright Epic Games, Inc. All Rights Reserved.

#include "RegistrySettings/LyraGameSettingRegistry.h"

#include "GameSettingCollection.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
// #include "EditCondition/WhenPlatformHasTrait.h"
#include "Settings/LyraSettingsLocal.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "RegistrySettings/PlayerSharedSettings.h"
// #include "Replays/LyraReplaySubsystem.h"
#define LOCTEXT_NAMESPACE "Lyra"


UGameSettingCollection* ULyraGameSettingRegistry::InitializeGameplaySettings(ULocalPlayer* InLocalPlayer)
{
	const auto Screen = UGameSettingCollection::CreateCollection("GameplayCollection",
	                                                             LOCTEXT("GameplayCollection_Name", "Gameplay"));
	Screen->Initialize(InLocalPlayer);

	const auto LanguageSubsection = UGameSettingCollection::CreateCollection(
		"LanguageCollection", LOCTEXT("LanguageCollection_Name", "Language"));
	Screen->AddSetting(LanguageSubsection);

	const auto Language = SetLanguageSettings(InLocalPlayer);
	LanguageSubsection->AddSetting(Language);

	const auto ReplaySubsection = UGameSettingCollection::CreateCollection(
		"ReplayCollection", LOCTEXT("ReplayCollection_Name", "Replays"));
	Screen->AddSetting(ReplaySubsection);

	const auto Replay = SetReplaySettings(InLocalPlayer);
	ReplaySubsection->AddSetting(Replay);
	const auto ReplayLimit = SetReplayLimitSettings(InLocalPlayer);
	ReplaySubsection->AddSetting(ReplayLimit);

	return Screen;
}


UGameSettingCollectionPage* ULyraGameSettingRegistry::SetLanguageSettings(ULocalPlayer* InLocalPlayer)
{
	auto LanguageSetting_Description = LOCTEXT("LanguageSetting_Description",
	                                           "Configure the language of the game.");
#if WITH_EDITOR
	if (GIsEditor)
		LanguageSetting_Description = LOCTEXT("LanguageSetting_WithEditor_Description",
		                                      "The language of the game.\n\n<text color=\"#ffff00\">WARNING:"
		                                      " Language changes will not affect PIE, you'll need to run with -game to test this,"
		                                      " or change your PIE language options in the editor preferences.</>");
#endif

	return UGameSettingCollectionPage::CreateSettings(
		"Language",
		LOCTEXT("LanguageSetting_Name", "Language"),
		LanguageSetting_Description,
		LOCTEXT("LanguageSetting", "Options"),
		FWhenPlayingAsPrimaryPlayer::Get());
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::SetReplaySettings(ULocalPlayer* InLocalPlayer)
{
	const auto RecordReplaySetting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		"RecordReplay",
		LOCTEXT("RecordReplaySetting_Name", "Record Replays"),
		LOCTEXT("RecordReplaySetting_Description",
		        "Automatically record game replays. Experimental feature, "
		        "recorded demos may have playback issues."),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(ShouldAutoRecordReplays),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(SetShouldAutoRecordReplays),
		GetDefault<ULyraSettingsLocal>()->ShouldAutoRecordReplays()
	);
	//@TODO - Replay - Add platform support trait
	// Setting->AddEditCondition(FWhenPlatformHasTrait::KillIfMissing(
	// 	ULyraReplaySubsystem::GetPlatformSupportTraitTag(), TEXT("Platform does not support saving replays")));

	RecordReplaySetting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	return RecordReplaySetting;
}

UGameSettingValueDiscreteDynamic_Number* ULyraGameSettingRegistry::SetReplayLimitSettings(
	ULocalPlayer* InLocalPlayer)
{
	const auto KeepReplayLimit = UGameSettingValueDiscreteDynamic_Number::Create(
		"KeepReplayLimit",
		LOCTEXT("KeepReplayLimitSetting_Name", "Keep Replay Limit"),
		LOCTEXT("KeepReplayLimitSetting_Description",
		        "Number of saved replays to keep, set to 0 for infinite."),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(GetNumberOfReplaysToKeep),
		GET_LOCAL_SETTINGS_FUNCTION_PATH(SetNumberOfReplaysToKeep),
		GetDefault<ULyraSettingsLocal>()->GetNumberOfReplaysToKeep(),
		20
	);

	KeepReplayLimit->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());
	return KeepReplayLimit;
}

#undef LOCTEXT_NAMESPACE
