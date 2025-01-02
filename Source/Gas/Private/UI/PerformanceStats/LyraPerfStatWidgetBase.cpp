// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/PerformanceStats/LyraPerfStatWidgetBase.h"

#include "Engine/GameInstance.h"
#include "Performance/LyraPerformanceStatSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPerfStatWidgetBase)

//////////////////////////////////////////////////////////////////////
// ULyraPerfStatWidgetBase

ULyraPerfStatWidgetBase::ULyraPerfStatWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

double ULyraPerfStatWidgetBase::FetchStatValue()
{
	if (CachedStatSubsystem == nullptr)
	{
		if (const UWorld* World = GetWorld())
		{
			if (const UGameInstance* GameInstance = World->GetGameInstance())
			{
				CachedStatSubsystem = GameInstance->GetSubsystem<ULyraPerformanceStatSubsystem>();
			}
		}
	}

	if (CachedStatSubsystem) { return CachedStatSubsystem->GetCachedStat(StatToDisplay); }

	return 0.0;
}
