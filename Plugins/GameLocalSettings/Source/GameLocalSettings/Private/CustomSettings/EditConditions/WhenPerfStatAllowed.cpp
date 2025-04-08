// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSettings/EditConditions/WhenPerfStatAllowed.h"

#include "CommonUIVisibilitySubsystem.h"
#include "GameplayTagContainer.h"
#include "Performance/LyraPerformanceSettings.h"


TSharedRef<FWhenPerfStatAllowed> FWhenPerfStatAllowed::KillIfNot(const EDisplayablePerformanceStat InAssociatedStat)
{
	static TSharedRef<FWhenPerfStatAllowed> Instance = MakeShared<FWhenPerfStatAllowed>();
	Instance->AssociatedStat = InAssociatedStat;
	return Instance;
}


void FWhenPerfStatAllowed::GatherEditState(const ULocalPlayer* InLocalPlayer, FGameSettingEditableState& InOutEditState) const
{
	const FGameplayTagContainer& VisibilityTags = UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->
		GetVisibilityTags();

	bool bCanShowStat = false;
	for (const FLyraPerformanceStatGroup& Group : GetDefault<ULyraPerformanceSettings>()->
	     UserFacingPerformanceStats) //@TODO: Move this stuff to per-platform instead of doing vis queries too?
	{
		if (!Group.AllowedStats.Contains(AssociatedStat)) continue;

		const bool bShowGroup = (Group.VisibilityQuery.IsEmpty() || Group.VisibilityQuery.Matches(
			VisibilityTags));

		if (bShowGroup)
		{
			bCanShowStat = true;
			break;
		}
	}

	if (!bCanShowStat)
	{
		InOutEditState.Hide(
			TEXT("Stat is not listed in ULyraPerformanceSettings or is suppressed by current platform traits"));
	}
}
