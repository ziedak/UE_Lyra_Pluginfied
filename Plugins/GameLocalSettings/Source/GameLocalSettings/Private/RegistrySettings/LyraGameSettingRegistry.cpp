// Copyright Epic Games, Inc. All Rights Reserved.

#include "RegistrySettings/LyraGameSettingRegistry.h"

#include "GameSettingCollection.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
// #include "Player/LyraLocalPlayer.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"
#include "UObject/EnumProperty.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameSettingRegistry)

namespace {}

DEFINE_LOG_CATEGORY(LogLyraGameSettingRegistry);

#define LOCTEXT_NAMESPACE "Lyra"

//--------------------------------------
// ULyraGameSettingRegistry
//--------------------------------------

ULyraGameSettingRegistry* ULyraGameSettingRegistry::Get(ULocalPlayer* InLocalPlayer)
{
	ULyraGameSettingRegistry* Registry = FindObject<ULyraGameSettingRegistry>(
		InLocalPlayer, TEXT("LyraGameSettingRegistry"), true);

	if (!Registry)
	{
		Registry = NewObject<ULyraGameSettingRegistry>(InLocalPlayer, TEXT("LyraGameSettingRegistry"));
		Registry->Initialize(InLocalPlayer);
	}

	return Registry;
}

bool ULyraGameSettingRegistry::IsFinishedInitializing() const
{
	if (!Super::IsFinishedInitializing()) { return false; }

	if (!OwningLocalPlayer || !OwningLocalPlayer->Implements<UPlayerSharedSettingsInterface>())
	{
		UE_LOG(LogLyraGameSettingRegistry, Error, TEXT("OwningLocalPlayer does not Implement IPlayerSharedSettings"));
		return true;
	}

	if (const auto ISharedSettings = Cast<IPlayerSharedSettingsInterface>(OwningLocalPlayer))
	{
		return ISharedSettings->GetSharedSettings() == nullptr;
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

	if (!OwningLocalPlayer || !OwningLocalPlayer->Implements<UPlayerSharedSettingsInterface>())
	{
		UE_LOG(LogLyraGameSettingRegistry, Error, TEXT("OwningLocalPlayer does not Implement IPlayerSharedSettings"));
		return;
	}

	if (const auto ISharedSettings = Cast<IPlayerSharedSettingsInterface>(OwningLocalPlayer))
	{
		ISharedSettings->GetLocalSettings()->ApplySettings(false);
		ISharedSettings->GetSharedSettings()->ApplySettings();
		ISharedSettings->GetSharedSettings()->SaveSettings();
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
