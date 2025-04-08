// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameSettingFilterState.h"

#include "Performance/LyraPerformanceStatTypes.h"

class ULocalPlayer;
/**
 * Checks the platform-specific value for FramePacingMode
 */

class FWhenFramePacingModeMatch : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenFramePacingModeMatch> KillIfMatch(EFramePacingMode InDesiredMode,
	                                                         EFramePacingEditCondition InMatchMode =
		                                                         EFramePacingEditCondition::EnableIf);


	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override;

private:
	EFramePacingMode DesiredMode = EFramePacingMode::DesktopStyle;
	EFramePacingEditCondition MatchMode = EFramePacingEditCondition::EnableIf;
};
