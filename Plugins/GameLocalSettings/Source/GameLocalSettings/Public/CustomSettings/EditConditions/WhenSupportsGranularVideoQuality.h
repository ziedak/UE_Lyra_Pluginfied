// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingFilterState.h"

class ULocalPlayer;
/**
 * Checks the platform-specific value for bSupportsGranularVideoQualitySettings
 */
class FWhenSupportsGranularVideoQuality : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenSupportsGranularVideoQuality> KillIfNotSupported(FString InDisableString);
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override;


	virtual void SettingChanged(const ULocalPlayer* LocalPlayer, UGameSetting* Setting,
	                            EGameSettingChangeReason Reason) const override;

private:
	FString DisableString;
};
