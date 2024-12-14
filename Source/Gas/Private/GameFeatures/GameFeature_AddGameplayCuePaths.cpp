// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeatures/GameFeature_AddGameplayCuePaths.h"

#include "Global/BaseGameplayCueManager.h"
#include "GameFeatureData.h"
#include "GameplayCueSet.h"
#include "AbilitySystemGlobals.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatures/GameFeatureAction_AddGameplayCuePath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeature_AddGameplayCuePaths)


void UGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData,
                                                                const FString& PluginName,
                                                                const FString& PluginURL)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering);

	const FString PluginRootPath = TEXT("/") + PluginName;
	for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGfa = Cast<
			UGameFeatureAction_AddGameplayCuePath>(Action);

		if (!AddGameplayCueGfa)
		{
			continue;
		}

		const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCueGfa->GetDirectoryPathsToAdd();
		UBaseGameplayCueManager* Gcm = UBaseGameplayCueManager::Get();
		if (!Gcm)
		{
			continue;
		}

		const UGameplayCueSet* RuntimeGameplayCueSet = Gcm->GetRuntimeCueSet();
		const int32 PreInitializeNumCues = RuntimeGameplayCueSet
			                                   ? RuntimeGameplayCueSet->GameplayCueData.Num()
			                                   : 0;

		// Add the new paths
		for (const auto& [Path] : DirsToAdd)
		{
			FString MutablePath = Path;
			UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
			Gcm->AddGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);
		}

		// Rebuild the runtime library with these new paths
		if (!DirsToAdd.IsEmpty())
		{
			Gcm->InitializeRuntimeObjectLibrary();
		}
		// Rebuild the runtime library with these new paths
		const int32 PostInitializeNumCues = RuntimeGameplayCueSet
			                                    ? RuntimeGameplayCueSet->GameplayCueData.Num()
			                                    : 0;

		if (PreInitializeNumCues != PostInitializeNumCues)
		{
			Gcm->RefreshGameplayCuePrimaryAsset();
		}
	}
}

void UGameFeature_AddGameplayCuePaths::OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData,
                                                                  const FString& PluginName, const FString& PluginURL)
{
	const FString PluginRootPath = TEXT("/") + PluginName;
	for (const UGameFeatureAction* Action : GameFeatureData->GetActions())
	{
		const UGameFeatureAction_AddGameplayCuePath* AddGameplayCueGFA = Cast<
			UGameFeatureAction_AddGameplayCuePath>(Action);
		if (!AddGameplayCueGFA)
		{
			continue;
		}

		const TArray<FDirectoryPath>& DirsToAdd = AddGameplayCueGFA->GetDirectoryPathsToAdd();
		UGameplayCueManager* Gcm = UAbilitySystemGlobals::Get().GetGameplayCueManager();

		if (!Gcm)
		{
			continue;
		}

		int32 NumRemoved = 0;
		for (const auto& [Path] : DirsToAdd)
		{
			FString MutablePath = Path;
			UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
			NumRemoved += Gcm->RemoveGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);
		}

		ensure(NumRemoved == DirsToAdd.Num());

		// Rebuild the runtime library only if there is a need to
		if (NumRemoved > 0)
		{
			Gcm->InitializeRuntimeObjectLibrary();
		}
	}
}
