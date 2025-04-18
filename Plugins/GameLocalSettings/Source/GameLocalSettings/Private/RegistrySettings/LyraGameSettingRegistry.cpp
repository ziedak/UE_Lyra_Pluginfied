// Copyright Epic Games, Inc. All Rights Reserved.

#include "RegistrySettings/LyraGameSettingRegistry.h"

#include "GameSettingCollection.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
// #include "Player/LyraLocalPlayer.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"
#include "UObject/EnumProperty.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameSettingRegistry)


DEFINE_LOG_CATEGORY(LogLyraGameSettingRegistry);

#define LOCTEXT_NAMESPACE "Lyra"

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
	// Check if the base class initialization is complete
	if (!Super::IsFinishedInitializing()) return false;

	// Validate that the owning local player implements the required interface
	if (!IsOwningLocalPlayerValid()) return false;

	// Check if shared settings are available
	if (!AreSharedSettingsAvailable()) return false;

	// All checks passed, initialization is complete
	return true;
}

bool ULyraGameSettingRegistry::IsOwningLocalPlayerValid() const
{
	if (!OwningLocalPlayer || !OwningLocalPlayer->Implements<UPlayerSharedSettingsInterface>())
	{
		UE_LOG(LogLyraGameSettingRegistry, Error, TEXT("OwningLocalPlayer does not Implement IPlayerSharedSettings"));
		return false;
	}
	return true;
}

bool ULyraGameSettingRegistry::AreSharedSettingsAvailable() const
{
	if (const auto ISharedSettings = Cast<IPlayerSharedSettingsInterface>(OwningLocalPlayer))
	{
		return ISharedSettings->GetSharedSettings() != nullptr;
	}
	return false;
}



void ULyraGameSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	VideoSettings = InitializeVideoSettings(InLocalPlayer);
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

	if (!IsOwningLocalPlayerValid())
	{
		return;
	}

	// verify this is the right way to do this
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