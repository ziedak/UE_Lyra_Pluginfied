// Copyright Epic Games, Inc. All Rights Reserved.

#include "RegistrySettings/LyraGameSettingRegistry.h"

#include "GameSettingCollection.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
// #include "Player/LyraLocalPlayer.h"
#include "RegistrySettings/PlayerSharedSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameSettingRegistry)

DEFINE_LOG_CATEGORY(LogLyraGameSettingRegistry);

#define LOCTEXT_NAMESPACE "Lyra"

//--------------------------------------
// ULyraGameSettingRegistry
//--------------------------------------

ULyraGameSettingRegistry::ULyraGameSettingRegistry()
{
}

ULyraGameSettingRegistry* ULyraGameSettingRegistry::Get(ULocalPlayer* InLocalPlayer)
{
	ULyraGameSettingRegistry* Registry = FindObject<ULyraGameSettingRegistry>(
		InLocalPlayer, TEXT("LyraGameSettingRegistry"), true);
	if (Registry == nullptr)
	{
		Registry = NewObject<ULyraGameSettingRegistry>(InLocalPlayer, TEXT("LyraGameSettingRegistry"));
		Registry->Initialize(InLocalPlayer);
	}

	return Registry;
}

bool ULyraGameSettingRegistry::IsFinishedInitializing() const
{
	if (!Super::IsFinishedInitializing())
		return false;

	if (OwningLocalPlayer && OwningLocalPlayer->Implements<UPlayerSharedSettings>())
	{
		if (const auto ISharedSettings = Cast<IPlayerSharedSettings>(OwningLocalPlayer))
		{
			if (!ISharedSettings->GetSharedSettings())
				return false;
		}
	}

	// ULyraLocalPlayer* LocalPlayer = Cast<ULyraLocalPlayer>(OwningLocalPlayer);
	// if (LocalPlayer && !LocalPlayer->GetSharedSettings())
	// 	return false;

	return true;
}

void ULyraGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	VideoSettings = InitializeVideoSettings(InLocalPlayer);
	InitializeVideoSettings_FrameRates(VideoSettings, InLocalPlayer);
	RegisterSetting(VideoSettings);

	AudioSettings = InitializeAudioSettings(InLocalPlayer);
	RegisterSetting(AudioSettings);

	GameplaySettings = InitializeGameplaySettings(InLocalPlayer);
	RegisterSetting(GameplaySettings);

	MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings(InLocalPlayer);
	RegisterSetting(MouseAndKeyboardSettings);

	GamepadSettings = InitializeGamepadSettings(InLocalPlayer);
	RegisterSetting(GamepadSettings);
}

void ULyraGameSettingRegistry::SaveChanges()
{
	Super::SaveChanges();

	if (OwningLocalPlayer && OwningLocalPlayer->Implements<UPlayerSharedSettings>())
	{
		if (const auto ISharedSettings = Cast<IPlayerSharedSettings>(OwningLocalPlayer))
		{
			ISharedSettings->GetLocalSettings()->ApplySettings(false);
			ISharedSettings->GetSharedSettings()->ApplySettings();
			ISharedSettings->GetSharedSettings()->SaveSettings();
		}
	}
	// if (ULyraLocalPlayer* LocalPlayer = Cast<ULyraLocalPlayer>(OwningLocalPlayer))
	// {
	// 	// Game user settings need to be applied to handle things like resolution, this saves indirectly
	// 	LocalPlayer->GetLocalSettings()->ApplySettings(false);
	// 	LocalPlayer->GetSharedSettings()->ApplySettings();
	// 	LocalPlayer->GetSharedSettings()->SaveSettings();
	// }
}

#undef LOCTEXT_NAMESPACE
