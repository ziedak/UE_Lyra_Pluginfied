#include "AssetManager/BaseAssetManager.h"
#include "AssetManager/BaseAssetManagerStartupJob.h"

// #include "Data/GasGameData.h"
// #include "Data/GasPawnData.h"
#include "Log/Loggger.h"
// #include "Global/BaseGameplayCueManager.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/App.h"
#include "Engine/Engine.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseAssetManager)

static FAutoConsoleCommand CVarDumpLoadedAssets(
	TEXT("Lyra.DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(UBaseAssetManager::DumpLoadedAssets)
	);

UBaseAssetManager& UBaseAssetManager::Get()
{
	check(GEngine);

	if (GEngine->AssetManager)
	{
		if (UBaseAssetManager* Singleton = Cast<UBaseAssetManager>(GEngine->AssetManager))
		{
			return *Singleton;
		}
	}

	ULOG_FATAL(LogGAS, "Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to BaseAssetManager!");

	// Fatal error above prevents this from being called.
	return *NewObject<UBaseAssetManager>();
}

void UBaseAssetManager::DumpLoadedAssets()
{
	ULOG_INFO(LogGAS, "========== Start Dumping Loaded Assets ==========");

	for (const UObject* LoadedAsset : Get().LoadedAssets)
		ULOG_INFO(LogGAS, "%s", *GetNameSafe(LoadedAsset));

	ULOG_INFO(LogGAS, "... %d assets unloaded pool", Get().LoadedAssets.Num());
	ULOG_INFO(LogGAS, "========== Finish Dumping Loaded Assets ==========");
}

// const UGasGameData& UBaseAssetManager::GetGameData()
// {
// 	return GetOrLoadTypedGameData<UGasGameData>(BaseGameDataPath);
// }
//
// const UGasPawnData* UBaseAssetManager::GetDefaultPawnData() const
// {
// 	return GetAsset(DefaultPawnData);
// }

/**
 * Synchronously loads an asset given its soft object path.
 *
 * @param AssetPath The soft object path of the asset to load.
 * @return The loaded asset object, or nullptr if the asset path is invalid or the asset couldn't be loaded.
 */
UObject* UBaseAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (!AssetPath.IsValid())
	{
		return nullptr;
	}

	const FString AssetName = AssetPath.GetAssetName();
	const FString AssetPathString = AssetPath.ToString();
	//FString AssetPackageName = AssetPath.GetLongPackageName();
	if (ShouldLogAssetLoads())
	{
		// * Allocates a new object of type T with the given arguments and returns it as a TUniquePtr.
		// Disabled for array-type TUniquePtrs.
		// * The object is value-initialized, which will call a user-defined default constructor if it exists, but a trivial type will be zeroed.

		TUniquePtr<FScopeLogTime> LoadTimePtr = MakeUnique<FScopeLogTime>(
			*FString::Printf(TEXT("Synchronous Load Asset: %s Path: %s"), *AssetName, *AssetPathString),
			nullptr,
			FScopeLogTime::ScopeLog_Seconds);
	}

	if (IsInitialized())
	{
		if (UObject* LoadedAsset = GetStreamableManager().LoadSynchronous(
			AssetPath, ShouldLogAssetLoads()))
		{
			return LoadedAsset;
		}
	}
	// Use LoadObject if asset manager isn't ready yet.
	return AssetPath.TryLoad();
}

bool UBaseAssetManager::ShouldLogAssetLoads()
{
	// Check if a command - line parameter named "AssetLoadLogging" is present when the program is executed.
	// This static variable "bAssetLoadLogging" can be accessed and used throughout the code to determine whether
	// asset load logging should be performed or not.

	static bool bAssetLoadLogging = FParse::Param(FCommandLine::Get(), TEXT("AssetLoadLogging"));
	return bAssetLoadLogging;
}

void UBaseAssetManager::AddLoadedAsset(const UObject* Asset)
{
	FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
	if (!ensureAlways(Asset))
	{
		return;
	}
	// Lock the critical section to prevent multiple threads from accessing the loaded assets array at the same time.
	LoadedAssets.Add(Asset);
}
#pragma region UAssetManager interface
// void UBaseAssetManager::StartInitialLoading()
// {
// 	// SCOPED_BOOT_TIMING is a macro invocation used to measure and log the time taken by the StartInitialLoading
// 	// function in the UBaseAssetManager class.
// 	// This macro is part of a performance profiling system that helps developers understand the time complexity
// 	// and performance characteristics of their code.
// 	SCOPED_BOOT_TIMING("UBaseAssetManager::StartInitialLoading");
//
// 	// This does all the scanning, need to do this now even if loads are deferred
// 	// because we need to know what assets are available
// 	Super::StartInitialLoading();
//
// 	// The STARTUP_JOB_WEIGHTED macro schedules the DoAllStartupJobs function to be executed as part of the
// 	// startup process, with the weight indicating its relative importance or the amount of time it is expected to take.
// 	// The weight can be used to manage and balance the execution of multiple startup jobs, ensuring that more critical
// 	// or time-consuming tasks are given appropriate priority.  In summary, this code snippet schedules the DoAllStartupJobs function to load base game data and store it in the GameDataMap, with a specified weight to manage its execution priority during the startup process.
// 	// StartupJobs.Add
// 	STARTUP_JOB(InitializeGameplayCueManager());
// 	//Load Base game data and store it in the GameDataMap
// 	STARTUP_JOB_WEIGHTED(GetGameData(), 25.f);
//
//
// 	// Run all the queued up startup jobs
// 	DoAllStartupJobs();
// }

UPrimaryDataAsset* UBaseAssetManager::LoadGameDataOfClass(
	const TSubclassOf<UPrimaryDataAsset>& DataClass,
	const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath,
	const FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* AssetToLoad = nullptr;
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UBaseAssetManager::LoadGameDataOfClass  : Loading GameData Object"),
	                            STAT__GameData, STATGROUP_LoadTime);

	if (DataClassPath.IsNull())
	{
		// It is not acceptable to fail to load any GameData asset. It will result in soft failures that are hard to diagnose.
		ULOG_ERROR(LogGAS, "Invalid GameData path for class %s", *DataClass->GetName());
		ULOG_ERROR(
			LogGAS,
			"Failed to load GameData asset at %s.Type %s.This is not recoverable and likely means you do not have the correct data to run %s.",
			*DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());

		return nullptr;
	}

#if WITH_EDITOR
	FScopedSlowTask SlowTask(0, FText::Format(
		                         NSLOCTEXT("GASEditor", "BeginLoadingGameDataTask", "Loading GameData %s"),
		                         FText::FromName(DataClass->GetFName())));
	constexpr bool bShowCancelButton = false;
	constexpr bool bAllowInPIE = true;
	SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif

	ULOG_INFO(LogGAS, "Loading GameData : %s ...", *DataClassPath.ToString());
	SCOPE_LOG_TIME_IN_SECONDS(TEXT("    ... GameData loaded!"), nullptr);

	// this can be called recursively, int the editor because it is called on demand so force a sync load for prinmary data assetsaand async load
	// the rest in that case

	// Load the GameData asset synchronously in the editor
	if (GIsEditor)
	{
		AssetToLoad = DataClassPath.LoadSynchronous();
		LoadPrimaryAssetsWithType(PrimaryAssetType);
	}
	else
	{
		// Load the GameData asset asynchronously and wait for it to complete

		const TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
		if (Handle.IsValid())
		{
			Handle->WaitUntilComplete(0, false);

			//this should always work since we just loaded it
			AssetToLoad = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
		}
	}

	if (!AssetToLoad)
	{
		ULOG_WARNING(LogGAS, "Failed to load GameData of class %s from path %s", *DataClass->GetName(),
		             *DataClassPath.ToString());
		return nullptr;
	}

	// Add the loaded data to the GameDataMap so we can find it later

	GameDataMap.Add(DataClass.Get(), AssetToLoad);

	return AssetToLoad;
}

void UBaseAssetManager::DoAllStartupJobs()
{
	SCOPED_BOOT_TIMING("UBaseAssetManager::DoAllStartupJobs");
	const double AllStartupJobsStartTime = FPlatformTime::Seconds();
	if (IsRunningDedicatedServer())
	{
		// no need for periodic progress updates, just run the jobs
		for (FBaseAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			StartupJob.DoJob();
		}
	}
	else
	{
		if (StartupJobs.Num() <= 0)
		{
			UpdateInitialGameContentLoadPercent(1.0f);
			return;
		}

		// If we have jobs, run them and update progress (if we have a delegate bound)
		// Calculate the total weight of all jobs
		float TotalJobValue = 0.0f;
		for (const FBaseAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			TotalJobValue += StartupJob.JobWeight;
		}

		float AccumulatedJobValue = 0.0f;
		// Run all the jobs and update progress as we go along	(if we have a delegate bound)
		for (FBaseAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			const float JobValue = StartupJob.JobWeight;
			// Bind a lambda to the substep delegate to update the progress bar
			StartupJob.SubstepProgressDelegate.BindLambda(
				[This = this, AccumulatedJobValue, JobValue, TotalJobValue](const float NewProgress)
				{
					const float SubstepAdjustment = FMath::Clamp(NewProgress, 0.0f, 1.0f) * JobValue;
					const float OverAllPercentWithSubstep = (AccumulatedJobValue + SubstepAdjustment) / TotalJobValue;

					This->UpdateInitialGameContentLoadPercent(OverAllPercentWithSubstep);
				});

			StartupJob.DoJob();

			StartupJob.SubstepProgressDelegate.Unbind();
			AccumulatedJobValue += JobValue;
			UpdateInitialGameContentLoadPercent(AccumulatedJobValue / TotalJobValue);
		}
	}

	StartupJobs.Empty();
	ULOG_INFO(LogGAS, "All startup jobs took %.2f seconds to complete",
	          FPlatformTime::Seconds() - AllStartupJobsStartTime);
}

void UBaseAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	// Could route this to the early startup loading screen
}

// void UBaseAssetManager::InitializeGameplayCueManager() const
// {
// 	SCOPED_BOOT_TIMING("UBaseAssetManager::InitializeGameplayCueManager");
//
// 	// Load the GameplayCueManager
// 	UBaseGameplayCueManager* Gcm = UBaseGameplayCueManager::Get();
// 	check(Gcm);
// 	Gcm->LoadAlwaysLoadedCues();
// }


#pragma endregion
