// Copyright Epic Games, Inc. All Rights Reserved.

#include "Settings/LyraSettingsShared.h"

#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Culture.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Rendering/SlateRenderer.h"
#include "SubtitleDisplaySubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSettingsShared)

static FString SHARED_SETTINGS_SLOT_NAME = TEXT("SharedGameSettings");

namespace LyraSettingsSharedCVars
{
	static float DefaultGamepadLeftStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadLeftStickInnerDeadZone(
		TEXT("gpad.DefaultLeftStickInnerDeadZone"),
		DefaultGamepadLeftStickInnerDeadZone,
		TEXT("Gamepad left stick inner deadzone")
	);

	static float DefaultGamepadRightStickInnerDeadZone = 0.25f;
	static FAutoConsoleVariableRef CVarGamepadRightStickInnerDeadZone(
		TEXT("gpad.DefaultRightStickInnerDeadZone"),
		DefaultGamepadRightStickInnerDeadZone,
		TEXT("Gamepad right stick inner deadzone")
	);
}

ULyraSettingsShared::ULyraSettingsShared()
{
	FInternationalization::Get().OnCultureChanged().AddUObject(this, &ThisClass::OnCultureChanged);

	GamepadMoveStickDeadZone = LyraSettingsSharedCVars::DefaultGamepadLeftStickInnerDeadZone;
	GamepadLookStickDeadZone = LyraSettingsSharedCVars::DefaultGamepadRightStickInnerDeadZone;
}

ULyraSettingsShared* ULyraSettingsShared::CreateTemporarySettings(const ULocalPlayer* LocalPlayer)
{
	// This is not loaded from disk but should be set up to save
	ULyraSettingsShared* SharedSettings = Cast<ULyraSettingsShared>(
		CreateNewSaveGameForLocalPlayer(StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

ULyraSettingsShared* ULyraSettingsShared::LoadOrCreateSettings(const ULocalPlayer* LocalPlayer)
{
	// This will stall the main thread while it loads
	ULyraSettingsShared* SharedSettings = Cast<ULyraSettingsShared>(
		LoadOrCreateSaveGameForLocalPlayer(StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

bool ULyraSettingsShared::AsyncLoadOrCreateSettings(const ULocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate)
{
	const FOnLocalPlayerSaveGameLoadedNative Lambda = FOnLocalPlayerSaveGameLoadedNative::CreateLambda([Delegate]
	(ULocalPlayerSaveGame* LoadedSave)
		{
			ULyraSettingsShared* LoadedSettings = CastChecked<ULyraSettingsShared>(LoadedSave);

			LoadedSettings->ApplySettings();

			Delegate.ExecuteIfBound(LoadedSettings);
		});

	return AsyncLoadOrCreateSaveGameForLocalPlayer(
		StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME, Lambda);
}

void ULyraSettingsShared::SaveSettings()
{
	// Schedule an async save because it's okay if it fails
	AsyncSaveGameToSlotForLocalPlayer();

	// TODO_BH: Move this to the serialize function instead with a bumped version number
	const auto EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(OwningPlayer);

	if (!EnhancedInputLocalPlayerSubsystem)
		return;
	if (UEnhancedInputUserSettings* InputSettings = EnhancedInputLocalPlayerSubsystem->GetUserSettings())
		InputSettings->AsyncSaveSettings();
}

void ULyraSettingsShared::ApplySettings()
{
	ApplySubtitleOptions();
	ApplyBackgroundAudioSettings();
	ApplyCultureSettings();

	ApplyInputSettings();
}

void ULyraSettingsShared::ApplyInputSettings() const
{
	const auto EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(OwningPlayer);

	if (!EnhancedInputLocalPlayerSubsystem)
		return;

	if (UEnhancedInputUserSettings* InputSettings = EnhancedInputLocalPlayerSubsystem->GetUserSettings())
		InputSettings->ApplySettings();
}

void ULyraSettingsShared::ApplySubtitleOptions() const
{
	if (USubtitleDisplaySubsystem* SubtitleSystem = USubtitleDisplaySubsystem::Get(OwningPlayer))
	{
		FSubtitleFormat SubtitleFormat;
		SubtitleFormat.SubtitleTextSize = SubtitleTextSize;
		SubtitleFormat.SubtitleTextColor = SubtitleTextColor;
		SubtitleFormat.SubtitleTextBorder = SubtitleTextBorder;
		SubtitleFormat.SubtitleBackgroundOpacity = SubtitleBackgroundOpacity;

		SubtitleSystem->SetSubtitleDisplayOptions(SubtitleFormat);
	}
}

//////////////////////////////////////////////////////////////////////

void ULyraSettingsShared::SetAllowAudioInBackgroundSetting(const EBackgroundAudioSetting NewValue)
{
	if (ChangeValueAndDirty(AllowAudioInBackground, NewValue))
		ApplyBackgroundAudioSettings();
}

void ULyraSettingsShared::ApplyBackgroundAudioSettings() const
{
	if (!OwningPlayer || !OwningPlayer->IsPrimaryPlayer()) return;

	const auto UnfocusedVolumeMultiplier = AllowAudioInBackground != EBackgroundAudioSetting::Off
		                                       ? 1.0f
		                                       : 0.0f;
	FApp::SetUnfocusedVolumeMultiplier(UnfocusedVolumeMultiplier);
}

void ULyraSettingsShared::ApplyCultureSettings()
{
	if (bResetToDefaultCulture)
	{
		ApplyDefaultCulture();
		return;
	}

	if (!PendingCulture.IsEmpty())
		ApplyPendingCulture();

	ClearPendingCulture();
}

void ULyraSettingsShared::ApplyDefaultCulture()
{
	const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
	check(SystemDefaultCulture.IsValid());

	const FString CultureToApply = SystemDefaultCulture->GetName();
	if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
	{
		// Clear this string
		GConfig->RemoveKey(TEXT("Internationalization"), TEXT("Culture"), GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
	bResetToDefaultCulture = false;
}

void ULyraSettingsShared::ApplyPendingCulture() const
{
	// SetCurrentCulture may trigger PendingCulture to be cleared (if a culture change is broadcast) so we take a copy of it to work with
	if (!FInternationalization::Get().SetCurrentCulture(PendingCulture))
		return;

	// Note: This is intentionally saved to the users config
	// We need to localize text before the player logs in and very early in the loading screen
	GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *PendingCulture, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void ULyraSettingsShared::ResetCultureToCurrentSettings()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

void ULyraSettingsShared::SetPendingCulture(const FString& NewCulture)
{
	PendingCulture = NewCulture;
	bResetToDefaultCulture = false;
	bIsDirty = true;
}

void ULyraSettingsShared::OnCultureChanged()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

bool ULyraSettingsShared::IsUsingDefaultCulture() const
{
	FString Culture;
	GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Culture, GGameUserSettingsIni);

	return Culture.IsEmpty();
}

void ULyraSettingsShared::ResetToDefaultCulture()
{
	ClearPendingCulture();
	bResetToDefaultCulture = true;
	bIsDirty = true;
}

//////////////////////////////////////////////////////////////////////

void ULyraSettingsShared::ApplyInputSensitivity() const
{
}

void ULyraSettingsShared::SetColorBlindStrength(int32 InColorBlindStrength)
{
	InColorBlindStrength = FMath::Clamp(InColorBlindStrength, 0, 10);
	if (ColorBlindStrength == InColorBlindStrength)
		return;

	ColorBlindStrength = InColorBlindStrength;
	FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
		static_cast<EColorVisionDeficiency>(static_cast<int32>(ColorBlindMode)),
		ColorBlindStrength,
		true,
		false);
}

void ULyraSettingsShared::SetColorBlindMode(const EColorBlindMode InMode)
{
	if (ColorBlindMode == InMode)
		return;

	ColorBlindMode = InMode;
	FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
		static_cast<EColorVisionDeficiency>(static_cast<int32>(ColorBlindMode)),
		ColorBlindStrength,
		true,
		false);
}
