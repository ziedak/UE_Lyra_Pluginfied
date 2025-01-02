// Fill out your copyright notice in the Description page of Project Settings.


#include "Experience/ExperienceManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Subsystems/SubsystemCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceManagerSubsystem)

#if WITH_EDITOR

void UExperienceManagerSubsystem::OnPlayInEditorBegun()
{
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void UExperienceManagerSubsystem::NotifyOfPluginActivation(const FString& PluginURL)
{
	if (GIsEditor)
	{
		const auto ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UExperienceManagerSubsystem>();
		check(ExperienceManagerSubsystem);

		// Track the number of requesters who activate this plugin. Multiple load/activation requests are always allowed because concurrent requests are handled.
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);
		++Count;
	}
}

bool UExperienceManagerSubsystem::RequestToDeactivatePlugin(const FString& PluginURL)
{
	if (!GIsEditor) { return true; }

	const auto ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<UExperienceManagerSubsystem>();
	check(ExperienceManagerSubsystem);

	// Only let the last requester to get this far deactivate the plugin
	int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
	--Count;

	if (Count != 0) { return false; }

	ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.Remove(PluginURL);
	return true;
}

#endif
