#pragma once

#include "Engine/AssetManager.h"
#include "AssetManager/BaseAssetManagerStartupJob.h"
#include "BaseAssetManager.generated.h"

class UPrimaryDataAsset;
class UObject;
struct FBaseAssetManagerStartupJob;
/**
 * UBaseAssetManager
 *
 *	Game implementation of the asset manager that overrides functionality and stores game-specific types.
 *	It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic.
 *	This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.

-- UCLASS(Config = Game) ==> This class can be configured in DefaultGame.ini
[/Script/GAS.BaseAssetManager]
BaseGameDataPath
DefaultPawnData
*/
UCLASS(Abstract, Config = Game)
class CUSTOMCORE_API UBaseAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UBaseAssetManager()
	{
	};

	// Returns the AssetManager singleton object.
	template <typename AssetManager>
	static AssetManager& Get2()
	{
		check(GEngine);

		if (GEngine->AssetManager)
		{
			if (AssetManager* Singleton = Cast<AssetManager>(GEngine->AssetManager))
			{
				return *Singleton;
			}
		}
		
		// Fatal error above prevents this from being called.
		return *NewObject<AssetManager>();
	}
	
	static UBaseAssetManager& Get();

	// Returns the asset referenced by a TSoftObjectPtr.  This will synchronously load the asset if it's not already loaded.
	template <typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// Returns the subclass referenced by a TSoftClassPtr.  This will synchronously load the asset if it's not already loaded.
	template <typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// Logs all assets currently loaded and tracked by the asset manager.
	static void DumpLoadedAssets();

	// const UGasGameData& GetGameData();
	//
	// const UGasPawnData* GetDefaultPawnData() const;

protected:
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		if (TObjectPtr<UPrimaryDataAsset> const* PResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*PResult);
		}
		
		// Does a blocking load if needed
		auto LoadedData = LoadGameDataOfClass(GameDataClass::StaticClass(),
																  DataPath,
																  GameDataClass::StaticClass()->GetFName());
		if (LoadedData)
		{
			return *CastChecked< GameDataClass>(LoadedData);
		}

		// Handle the case where the data could not be loaded
		UE_LOG(LogTemp, Error, TEXT("Failed to load game data of class %s"), *GameDataClass::StaticClass()->GetName());
		return *NewObject<GameDataClass>(); // Return a default instance to avoid returning a nullptr
	}

	static UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);
	static bool ShouldLogAssetLoads();

	// Thread safe way of adding a loaded asset to keep in memory.
	void AddLoadedAsset(const UObject* Asset);

// #pragma region UAssetManager interface
//
// 	// virtual void StartInitialLoading() override;
// #if WITH_EDITOR
// 	virtual void PreBeginPIE(bool bStartSimulate) override;
// #endif
//
// #pragma endregion

	UPrimaryDataAsset* LoadGameDataOfClass(const TSubclassOf<UPrimaryDataAsset>& DataClass,
	                                       const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath,
	                                       FPrimaryAssetType PrimaryAssetType);

protected:
	// // Global game data asset to use (DefaultGame.ini).
	// UPROPERTY(Config)
	// TSoftObjectPtr<UGasGameData> BaseGameDataPath;
	//
	// // Pawn data used when spawning player pawns if there isn't one set on the player state (DefaultGame.ini).
	// UPROPERTY(Config)
	// TSoftObjectPtr<UGasPawnData> DefaultPawnData;

	// Loaded version of the game data
	UPROPERTY(Transient)
	// This means that the property's value is not saved to disk and is not replicated over the network.
	TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;
protected:
	// Flushes the StartupJobs array. Processes all startup work.
	void DoAllStartupJobs();
	
	// The list of tasks to execute on startup. Used to track startup progress.
	TArray<FBaseAssetManagerStartupJob> StartupJobs;
private:

	// Sets up the ability system
	// void InitializeGameplayCueManager() const;

	// Called periodically during loads, could be used to feed the status to a loading screen
	void UpdateInitialGameContentLoadPercent(float GameContentPercent);
	
	// Assets loaded and tracked by the asset manager.
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	// Used for a scope lock when modifying the list of load assets.
	FCriticalSection LoadedAssetsCritical;
};

template <typename AssetType>
	AssetType* UBaseAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

template <typename AssetType>
TSubclassOf<AssetType> UBaseAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}
