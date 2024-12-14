// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GAssetManager.h"

#include "AssetManager/Startup_job.h"
#include "Global/BaseGameplayCueManager.h"
#include "Data/GasGameData.h"
#include "Data/GasPawnData.h"
#include "Log/Loggger.h"

UGAssetManager& UGAssetManager::Get()
{
	return Get2<UGAssetManager>();
}

void UGAssetManager::StartInitialLoading()
{
	// SCOPED_BOOT_TIMING is a macro invocation used to measure and log the time taken by the StartInitialLoading
	// function in the UBaseAssetManager class.
	// This macro is part of a performance profiling system that helps developers understand the time complexity
	// and performance characteristics of their code.
	SCOPED_BOOT_TIMING("UBaseAssetManager::StartInitialLoading");

	// This does all the scanning, need to do this now even if loads are deferred
	// because we need to know what assets are available
	Super::StartInitialLoading();

	// The STARTUP_JOB_WEIGHTED macro schedules the DoAllStartupJobs function to be executed as part of the
	// startup process, with the weight indicating its relative importance or the amount of time it is expected to take.
	// The weight can be used to manage and balance the execution of multiple startup jobs, ensuring that more critical
	// or time-consuming tasks are given appropriate priority.  In summary, this code snippet schedules the DoAllStartupJobs function to load base game data and store it in the GameDataMap, with a specified weight to manage its execution priority during the startup process.
	// StartupJobs.Add
	STARTUP_JOB(InitializeGameplayCueManager());
	//Load Base game data and store it in the GameDataMap
	STARTUP_JOB_WEIGHTED(GetGameData(), 25.f);


	// Run all the queued up startup jobs
	DoAllStartupJobs();
}

void UGAssetManager::InitializeGameplayCueManager() const
{
	SCOPED_BOOT_TIMING("UBaseAssetManager::InitializeGameplayCueManager");

	// Load the GameplayCueManager
	UBaseGameplayCueManager* Gcm = UBaseGameplayCueManager::Get();
	check(Gcm);
	Gcm->LoadAlwaysLoadedCues();
}

const UGasGameData& UGAssetManager::GetGameData()
{
	return GetOrLoadTypedGameData<UGasGameData>(BaseGameDataPath);
}

UGasPawnData* UGAssetManager::GetDefaultPawnData() const
{
	return GetAsset(DefaultPawnData);
}


#if WITH_EDITOR
//'PIE' mode refers to 'Play In Editor'.
void UGAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);
	{
		FScopedSlowTask SlowTask(0, NSLOCTEXT("GasEditor", "PreBeginPIE", "Loading PIE Data"));
		constexpr bool bShowCancelButton = false;
		constexpr bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);

		const UGasGameData& GameData = GetGameData();
		//SlowTask.EnterProgressFrame(1.f, FText::Format(NSLOCTEXT("Gaas Editor", "LoadingGameData", "Loading Game Data: {0}"), FText::FromString(GameData.GetName())));

		//Intentionally after GetGameData to avoid counting GameData time in the progress bar as it's already counted in the main progress bar
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBeginPIE"), nullptr);

		//You could add preloading of anythind else needed for the experience we will be using here
		// (e.g. loading of the default pawn data)
		//(e.g. by grabbing the default experience from the world settings + experience override in developper settings )
	}
}

#endif
