// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSettings/LyraSettingValueDiscrete_PerfStat.h"

#include "CommonUIVisibilitySubsystem.h"

#include "CustomSettings/EditConditions/WhenPerfStatAllowed.h"

#include "Performance/LyraPerformanceSettings.h"
#include "Performance/LyraPerformanceStatTypes.h"
#include "Settings/LyraSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSettingValueDiscrete_PerfStat)

class ULocalPlayer;

#define LOCTEXT_NAMESPACE "LyraSettings"

ULyraSettingValueDiscrete_PerfStat::ULyraSettingValueDiscrete_PerfStat(): StatToDisplay(), InitialMode() {}

void ULyraSettingValueDiscrete_PerfStat::SetStat(const EDisplayablePerformanceStat InStat)
{
	StatToDisplay = InStat;
	SetDevName(FName(*FString::Printf(TEXT("PerfStat_%d"), static_cast<int32>(StatToDisplay))));
	AddEditCondition(FWhenPerfStatAllowed::KillIfNot(StatToDisplay));
}

void ULyraSettingValueDiscrete_PerfStat::AddMode(FText&& Label, const EStatDisplayMode Mode)
{
	Options.Emplace(MoveTemp(Label));
	DisplayModes.Add(Mode);
}

void ULyraSettingValueDiscrete_PerfStat::OnInitialized()
{
	Super::OnInitialized();

	AddMode(LOCTEXT("PerfStatDisplayMode_None", "None"), EStatDisplayMode::Hidden);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextOnly", "Text Only"), EStatDisplayMode::TextOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_GraphOnly", "Graph Only"), EStatDisplayMode::GraphOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextAndGraph", "Text and Graph"), EStatDisplayMode::TextAndGraph);
}

void ULyraSettingValueDiscrete_PerfStat::StoreInitial()
{
	const ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	InitialMode = Settings->GetPerfStatDisplayState(StatToDisplay);
}

void ULyraSettingValueDiscrete_PerfStat::ResetToDefault()
{
	ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, EStatDisplayMode::Hidden);
	NotifySettingChanged(EGameSettingChangeReason::ResetToDefault);
}

void ULyraSettingValueDiscrete_PerfStat::RestoreToInitial()
{
	ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, InitialMode);
	NotifySettingChanged(EGameSettingChangeReason::RestoreToInitial);
}

void ULyraSettingValueDiscrete_PerfStat::SetDiscreteOptionByIndex(const int32 Index)
{
	if (DisplayModes.IsValidIndex(Index))
	{
		const EStatDisplayMode DisplayMode = DisplayModes[Index];

		ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
		Settings->SetPerfStatDisplayState(StatToDisplay, DisplayMode);
	}
	NotifySettingChanged(EGameSettingChangeReason::Change);
}

int32 ULyraSettingValueDiscrete_PerfStat::GetDiscreteOptionIndex() const
{
	const ULyraSettingsLocal* Settings = ULyraSettingsLocal::Get();
	return DisplayModes.Find(Settings->GetPerfStatDisplayState(StatToDisplay));
}


#undef LOCTEXT_NAMESPACE