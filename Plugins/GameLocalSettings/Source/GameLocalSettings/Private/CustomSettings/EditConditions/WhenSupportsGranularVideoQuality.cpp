// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSettings/EditConditions/WhenSupportsGranularVideoQuality.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"
#include "Performance/LyraPerformanceSettings.h"

#include "RegistrySettings/LyraGameSettingRegistry.h"

TSharedRef<FWhenSupportsGranularVideoQuality> FWhenSupportsGranularVideoQuality::KillIfNotSupported(FString InDisableString)
{
	check(!InDisableString.IsEmpty());
	static TSharedRef<FWhenSupportsGranularVideoQuality> Instance = MakeShared<FWhenSupportsGranularVideoQuality>();
	Instance->DisableString = InDisableString;
	return Instance;
}

void FWhenSupportsGranularVideoQuality::GatherEditState(const ULocalPlayer* InLocalPlayer,
                                                        FGameSettingEditableState& InOutEditState) const
{
	if (!ULyraPlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings) InOutEditState.Kill(DisableString);
}

void FWhenSupportsGranularVideoQuality::SettingChanged(const ULocalPlayer* LocalPlayer, UGameSetting* Setting,
                                                       EGameSettingChangeReason Reason) const
{
	// TODO for now this applies the setting immediately
	if (!LocalPlayer || !LocalPlayer->Implements<UPlayerSharedSettingsInterface>())
	{
		UE_LOG(LogLyraGameSettingRegistry, Error,
		       TEXT("OwningLocalPlayer does not Implement IPlayerSharedSettings"));
		return;
	}

	if (const auto ISharedSettings = CastChecked<IPlayerSharedSettingsInterface>(LocalPlayer)) ISharedSettings->GetLocalSettings()->ApplyScalabilitySettings();
	// const ULyraLocalPlayer* LyraLocalPlayer = CastChecked<ULyraLocalPlayer>(LocalPlayer);
	// LyraLocalPlayer->GetLocalSettings()->ApplyScalabilitySettings();
}
