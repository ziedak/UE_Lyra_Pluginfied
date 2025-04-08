// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameSettingFilterState.h"
#include "Performance/LyraPerformanceSettings.h"

/**
 *
 */
class FWhenPerfStatAllowed : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPerfStatAllowed> KillIfNot(const EDisplayablePerformanceStat InAssociatedStat);

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override;

private:
	EDisplayablePerformanceStat AssociatedStat = EDisplayablePerformanceStat::IdleTime;
};
