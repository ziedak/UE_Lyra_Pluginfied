// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSettings/EditConditions/WhenFramePacingModeMatch.h"
#include "Performance/LyraPerformanceSettings.h"

TSharedRef<FWhenFramePacingModeMatch> FWhenFramePacingModeMatch::KillIfMatch(const EFramePacingMode InDesiredMode,
                                                                             const EFramePacingEditCondition InMatchMode)
{
	static TSharedRef<FWhenFramePacingModeMatch> Instance = MakeShared<FWhenFramePacingModeMatch>();
	Instance->DesiredMode = InDesiredMode;
	Instance->MatchMode = InMatchMode;
	return Instance;
}

void FWhenFramePacingModeMatch::GatherEditState(const ULocalPlayer* InLocalPlayer,
                                                FGameSettingEditableState& InOutEditState) const
{
	const EFramePacingMode ActualMode = ULyraPlatformSpecificRenderingSettings::Get()->FramePacingMode;

	const bool bMatches = ActualMode == DesiredMode;
	const bool bMatchesAreBad = MatchMode == EFramePacingEditCondition::DisableIf;

	if (bMatches == bMatchesAreBad)
	{
		InOutEditState.Kill(FString::Printf(
			TEXT("Frame pacing mode %d didn't match requirement %d"), static_cast<int32>(ActualMode),
			static_cast<int32>(DesiredMode)));
	}
}
