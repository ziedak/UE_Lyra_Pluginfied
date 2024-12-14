#include "Global/BaseGameplayCueManager.h"

#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "GameplayCueSet.h"
#include "Engine/AssetManager.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGameplayCueManager)

enum class EEditorLoadMode
{
	// Loads all cues upfront; longer loading speed in the editor but short PIE times and effects never fail to play
	LoadUpfront,

	// Outside of editor: Async loads as cue tag are registered
	// In editor: Async loads when cues are invoked
	//   Note: This can cause some 'why didn't I see the effect for X' issues in PIE and is good for iteration speed but otherwise bad for designers
	PreloadAsCuesAreReferenced_GameOnly,

	// Async loads as cue tag are registered
	PreloadAsCuesAreReferenced
};

// Console variables for controlling how gameplay cues are loaded
namespace GameplayCueManagerCvars
{
	static FAutoConsoleCommand CVarDumpGameplayCues(
		TEXT("GAS.DumpGameplayCues"),
		TEXT("Shows all assets that were loaded via BaseGameplayCueManager and are currently in memory."),
		FConsoleCommandWithArgsDelegate::CreateStatic(UBaseGameplayCueManager::DumpGameplayCues));

	static EEditorLoadMode LoadMode = EEditorLoadMode::LoadUpfront;
}

constexpr bool bPreloadEvenInEditor = true;

//////////////////////////////////////////////////////////////////////

struct FGameplayCueTagThreadSynchronizeGraphTask : FAsyncGraphTaskBase
{
	TFunction<void()> TheTask;

	explicit FGameplayCueTagThreadSynchronizeGraphTask(TFunction<void()>&& Task) : TheTask(MoveTemp(Task))
	{
	}

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) const { TheTask(); }
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
};

//////////////////////////////////////////////////////////////////////

UBaseGameplayCueManager::UBaseGameplayCueManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}


// This function is responsible for initializing the gameplay cue manager.
UBaseGameplayCueManager* UBaseGameplayCueManager::Get()
{
	return Cast<UBaseGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
}

void UBaseGameplayCueManager::OnCreated()
{
	Super::OnCreated();

	UpdateDelayLoadDelegateListeners();
}

// This makes it so GameplayCues will load the first time that they're requested (or use your AssetManager to manually load them).
// By default, it loads *every* GameplayCue in the project and all of their referenced assets when the map starts. In a large game
// with lots of GCs, this could be hundreds of megabytes or more of unused assets loaded in RAM if particular GCs are not used in
// the current map.
bool UBaseGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
	switch (GameplayCueManagerCvars::LoadMode)
	{
	case EEditorLoadMode::LoadUpfront:
		return true;
	case EEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return false;
		}
#endif
		break;
	case EEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	//todo: verify this
	return !ShouldDelayLoadGameplayCues();
}

/** If true, this will synchronously load missing gameplay cues */
bool UBaseGameplayCueManager::ShouldSyncLoadMissingGameplayCues() const
{
	return false;
}

/** If true, this will asynchronously load missing gameplay cues, and execute cue when the load finishes */
bool UBaseGameplayCueManager::ShouldAsyncLoadMissingGameplayCues() const
{
	return true;
}

void UBaseGameplayCueManager::DumpGameplayCues(const TArray<FString>& Args)
{
	UBaseGameplayCueManager* Gcm = Cast<UBaseGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
	if (!Gcm)
	{
		UE_LOG(LogTemp, Error, TEXT("DumpGameplayCues failed. No UBaseGameplayCueManager found."));
		return;
	}

	const bool bIncludeRefs = Args.Contains(TEXT("Refs"));

	UE_LOG(LogTemp, Log, TEXT("=========== Dumping Always Loaded Gameplay Cue Notifies ==========="));
	for (const UClass* CueClass : Gcm->AlwaysLoadedCues)
	{
		UE_LOG(LogTemp, Log, TEXT("  %s"), *GetPathNameSafe(CueClass));
	}

	UE_LOG(LogTemp, Log, TEXT("=========== Dumping Preloaded Gameplay Cue Notifies ==========="));
	for (UClass* CueClass : Gcm->PreloadedCues)
	{
		TSet<FObjectKey>* ReferencerSet = Gcm->PreloadedCueReferencers.Find(CueClass);
		const int32 NumRefs = ReferencerSet ? ReferencerSet->Num() : 0;
		UE_LOG(LogTemp, Log, TEXT("  %s (%d refs)"), *GetPathNameSafe(CueClass), NumRefs);

		if (!bIncludeRefs || !ReferencerSet)
		{
			continue;
		}

		for (const FObjectKey& Ref : *ReferencerSet)
		{
			const UObject* RefObject = Ref.ResolveObjectPtr();
			UE_LOG(LogTemp, Log, TEXT("    ^- %s"), *GetPathNameSafe(RefObject));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("=========== Dumping Gameplay Cue Notifies loaded on demand ==========="));
	int32 NumMissingCuesLoaded = 0;
	if (Gcm->RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (const FGameplayCueNotifyData& CueData : Gcm->RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData)
		{
			if (!CueData.LoadedGameplayCueClass
			    || Gcm->AlwaysLoadedCues.Contains(CueData.LoadedGameplayCueClass)
			    || Gcm->PreloadedCues.Contains(CueData.LoadedGameplayCueClass))
			{
				continue;
			}

			NumMissingCuesLoaded++;
			UE_LOG(LogTemp, Log, TEXT("  %s"), *CueData.LoadedGameplayCueClass->GetPathName());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("=========== Gameplay Cue Notify summary ==========="));
	UE_LOG(LogTemp, Log, TEXT("  ... %d cues in always loaded list"), Gcm->AlwaysLoadedCues.Num());
	UE_LOG(LogTemp, Log, TEXT("  ... %d cues in preloaded list"), Gcm->PreloadedCues.Num());
	UE_LOG(LogTemp, Log, TEXT("  ... %d cues loaded on demand"), NumMissingCuesLoaded);
	UE_LOG(LogTemp, Log, TEXT("  ... %d cues in total"),
	       Gcm->AlwaysLoadedCues.Num() + Gcm->PreloadedCues.Num() + NumMissingCuesLoaded);
}

void UBaseGameplayCueManager::LoadAlwaysLoadedCues()
{
	if (!ShouldDelayLoadGameplayCues())
	{
		return;
	}

	const UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

	//@TODO: Try to collect these by filtering GameplayCue. tags out of native gameplay tags?
	TArray<FName> AdditionalAlwaysLoadedCueTags;

	for (const FName& CueTagName : AdditionalAlwaysLoadedCueTags)
	{
		FGameplayTag CueTag = TagManager.RequestGameplayTag(CueTagName, /*ErrorIfNotFound=*/ false);
		if (!CueTag.IsValid())
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("UBaseGameplayCueManager::AdditionalAlwaysLoadedCueTags contains invalid tag %s"),
			       *CueTagName.ToString());
			continue;
		}

		ProcessTagToPreload(CueTag, nullptr);
	}
}

const FPrimaryAssetType UFortAssetManager_GameplayCueRefsType = TEXT("GameplayCueRefs");
const FName UFortAssetManager_GameplayCueRefsName = TEXT("GameplayCueReferences");
const FName UFortAssetManager_LoadStateClient = FName(TEXT("Client"));
/*
* This function is responsible for updating the primary asset for the gameplay cues.
* It retrieves the paths of the gameplay cues from the runtime cue set and adds them to the asset bundle data.
* The primary asset ID is set to the gameplay cue references type and name, and the asset manager is used to add the dynamic asset.
* This function is called when the gameplay cue manager is created and when the gameplay cue primary asset needs to be refreshed.
* The primary asset is used to manage the gameplay cue references and ensure that they are loaded and available when needed.
* ========================================================================================================
* The function is called from the OnCreated() function of the gameplay cue manager to initialize the primary asset.
* It is also called from the RefreshGameplayCuePrimaryAsset() function to update the primary asset with the latest gameplay cue references.
*
*/
void UBaseGameplayCueManager::RefreshGameplayCuePrimaryAsset()
{
	TArray<FSoftObjectPath> CuePaths;
	if (const UGameplayCueSet* RuntimeGameplayCueSet = GetRuntimeCueSet())
	{
		RuntimeGameplayCueSet->GetSoftObjectPaths(CuePaths);
	}

	FAssetBundleData BundleData;
	BundleData.AddBundleAssetsTruncated(UFortAssetManager_LoadStateClient, CuePaths);

	const FPrimaryAssetId PrimaryAssetId = FPrimaryAssetId(UFortAssetManager_GameplayCueRefsType,
	                                                       UFortAssetManager_GameplayCueRefsName);
	UAssetManager::Get().AddDynamicAsset(PrimaryAssetId, FSoftObjectPath(), BundleData);
}

/*
* This function is called when a gameplay tag is loaded and is responsible for processing the loaded tags.
* Let's break down the code step by step:
* 1.	The function begins by acquiring a lock on the LoadedGameplayTagsToProcessCS critical section using FScopeLock.
*	This ensures that the access to the LoadedGameplayTagsToProcess array is synchronized and thread-safe.
* 2.	The function checks if there are any tags in the LoadedGameplayTagsToProcess array.
*	If the array is empty, it sets the bStartTask flag to true, indicating that a new task needs to be started.
* 3.	The function then retrieves the current serialize context using FUObjectThreadContext::Get().GetSerializeContext().
*	This context contains information about the current serialization operation, such as the serialized object.
* 4.	The function assigns the retrieved serialize context's serialized object to the OwningObject variable.
	This object represents the owner of the loaded gameplay tag.
* 5.	The function adds the loaded gameplay tag and its owning object to the LoadedGameplayTagsToProcess array using Emplace.
* 6.	If the bStartTask flag is true, indicating that a new task needs to be started,
*	the function creates a new graph task using TGraphTask<FGameplayCueTagThreadSynchronizeGraphTask>::CreateTask().ConstructAndDispatchWhenReady().
*	This task is responsible for processing the loaded tags.
* 7.	The created task is a lambda function that checks if the game is currently running (GIsRunning).
*	If it is, it retrieves a strong reference to the UBaseGameplayCueManager instance using Get().
* 8.	If the game is currently garbage collecting (IsGarbageCollecting()), the task sets the bProcessLoadedTagsAfterGC flag to true.
*	This flag indicates that the loaded tags should be processed after the garbage collection is complete.
* 9.	If the game is not garbage collecting, the task calls the ProcessLoadedTags() function on the UBaseGameplayCueManager instance to process the loaded tags immediately.
* 10.	The task is then dispatched to the game thread using ConstructAndDispatchWhenReady().
* ============================================================================
*	In summary, the OnGameplayTagLoaded function is responsible for handling the event when a gameplay tag is loaded.
*	It adds the loaded tag and its owning object to an array and starts a new task to process the loaded tags.
*	The processing of the tags is either done immediately or after the garbage collection is complete, depending on the current state of the game.
*/
void UBaseGameplayCueManager::OnGameplayTagLoaded(const FGameplayTag& Tag)
{
	FScopeLock ScopeLock(&LoadedGameplayTagsToProcessCS);
	const bool bStartTask = LoadedGameplayTagsToProcess.Num() == 0;
	const FUObjectSerializeContext* LoadContext = FUObjectThreadContext::Get().GetSerializeContext();
	UObject* OwningObject = LoadContext ? LoadContext->SerializedObject : nullptr;
	LoadedGameplayTagsToProcess.Emplace(Tag, OwningObject);

	if (!bStartTask)
	{
		return;
	}

	TGraphTask<FGameplayCueTagThreadSynchronizeGraphTask>::CreateTask().ConstructAndDispatchWhenReady([]()
	{
		if (GIsRunning)
		{
			if (UBaseGameplayCueManager* StrongThis = Get())
			{
				// If we are garbage collecting we cannot call StaticFindObject (or a few other static uobject functions), so we'll just wait until the GC is over and process the tags then
				if (IsGarbageCollecting())
				{
					StrongThis->bProcessLoadedTagsAfterGC = true;
				}
				else
				{
					StrongThis->ProcessLoadedTags();
				}
			}
		}
	});
}

void UBaseGameplayCueManager::HandlePostGarbageCollect()
{
	if (bProcessLoadedTagsAfterGC)
	{
		ProcessLoadedTags();
	}

	bProcessLoadedTagsAfterGC = false;
}

void UBaseGameplayCueManager::ProcessLoadedTags()
{
	TArray<FLoadedGameplayTagToProcessData> TaskLoadedGameplayTagsToProcess;
	{
		// Lock LoadedGameplayTagsToProcess just long enough to make a copy and clear
		FScopeLock TaskScopeLock(&LoadedGameplayTagsToProcessCS);
		TaskLoadedGameplayTagsToProcess = LoadedGameplayTagsToProcess;
		LoadedGameplayTagsToProcess.Empty();
	}

	// This might return during shutdown, and we don't want to proceed if that is the case
	if (!GIsRunning)
	{
		return;
	}

	if (!RuntimeGameplayCueObjectLibrary.CueSet)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "UBaseGameplayCueManager::OnGameplayTagLoaded processed loaded tag(s) but RuntimeGameplayCueObjectLibrary.CueSet was null. Skipping processing."
		       ));
		return;
	}

	for (const FLoadedGameplayTagToProcessData& LoadedTagData : TaskLoadedGameplayTagsToProcess)
	{
		if (!RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Contains(LoadedTagData.Tag) || LoadedTagData.
		    WeakOwner.IsStale())
		{
			continue;
		}

		ProcessTagToPreload(LoadedTagData.Tag, LoadedTagData.WeakOwner.Get());
	}
}

void UBaseGameplayCueManager::ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject)
{
	switch (GameplayCueManagerCvars::LoadMode)
	{
	case EEditorLoadMode::LoadUpfront:
		return;
	case EEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		break;
	case EEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	check(RuntimeGameplayCueObjectLibrary.CueSet);

	const int32* DataIdx = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Find(Tag);
	if (!DataIdx || !RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData.IsValidIndex(*DataIdx))
	{
		return;
	}

	const FGameplayCueNotifyData& CueData = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData[*DataIdx];

	if (UClass* LoadedGameplayCueClass = FindObject<UClass>(nullptr, *CueData.GameplayCueNotifyObj.ToString()))
	{
		RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject);
	}
	else
	{
		bool bAlwaysLoadedCue = OwningObject == nullptr;
		TWeakObjectPtr<UObject> WeakOwner = OwningObject;

		StreamableManager.RequestAsyncLoad(
			CueData.GameplayCueNotifyObj,
			FStreamableDelegate::CreateUObject(
				this,
				&ThisClass::OnPreloadCueComplete,
				CueData.GameplayCueNotifyObj,
				WeakOwner,
				bAlwaysLoadedCue
				),
			FStreamableManager::DefaultAsyncLoadPriority,
			false,
			false,
			TEXT("GameplayCueManager")
			);
	}
}

void UBaseGameplayCueManager::OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject,
                                                   bool bAlwaysLoadedCue)
{
	if (!bAlwaysLoadedCue && !OwningObject.IsValid())
	{
		return;
	}

	if (UClass* LoadedGameplayCueClass = Cast<UClass>(Path.ResolveObject()))
	{
		RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject.Get());
	}
}

/*
* 1.	The function then checks if the OwningObject parameter is nullptr, which indicates that the gameplay cue is always loaded. If it is,
*	the LoadedGameplayCueClass is added to the AlwaysLoadedCues set, and it is removed from the PreloadedCues set and the PreloadedCue References map.
* 2.	If the OwningObject is not nullptr, the function checks if it is different from both the LoadedGameplayCueClass and its default object. 
*	This ensures that the OwningObject is not the same as the gameplay cue class itself or its default object.
* 3.	If the above condition is true and the LoadedGameplayCueClass is not already in the AlwaysLoadedCues set,
*	the LoadedGameplayCueClass is added to the PreloadedCues set.
* 4.	The function then finds or adds the LoadedGameplayCueClass as a key in the PreloadedCue References map and adds the OwningObject to 
*	the corresponding set of references.
*	=======================================================================================================
*In summary, the RegisterPreloadedCue function is used to register preloaded gameplay cues and their references. 
*	It distinguishes between cues that are always loaded and cues that are preloaded based on the OwningObject parameter
*/
void UBaseGameplayCueManager::RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject)
{
	check(LoadedGameplayCueClass);

	const bool bAlwaysLoadedCue = OwningObject == nullptr;
	if (bAlwaysLoadedCue)
	{
		AlwaysLoadedCues.Add(LoadedGameplayCueClass);
		PreloadedCues.Remove(LoadedGameplayCueClass);
		PreloadedCueReferencers.Remove(LoadedGameplayCueClass);
	}
	else if ((OwningObject != LoadedGameplayCueClass)
	         && (OwningObject != LoadedGameplayCueClass->GetDefaultObject())
	         && !AlwaysLoadedCues.Contains(LoadedGameplayCueClass)
	)
	{
		PreloadedCues.Add(LoadedGameplayCueClass);
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindOrAdd(LoadedGameplayCueClass);
		ReferencerSet.Add(OwningObject);
	}
}

// 
void UBaseGameplayCueManager::HandlePostLoadMap(UWorld* NewWorld)
{
	if (RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (UClass* CueClass : AlwaysLoadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}

		for (UClass* CueClass : PreloadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}
	}

	for (auto CueIt = PreloadedCues.CreateIterator(); CueIt; ++CueIt)
	{
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindChecked(*CueIt);
		for (auto RefIt = ReferencerSet.CreateIterator(); RefIt; ++RefIt)
		{
			if (!RefIt->ResolveObjectPtr())
			{
				RefIt.RemoveCurrent();
			}
		}
		if (ReferencerSet.Num() == 0)
		{
			PreloadedCueReferencers.Remove(*CueIt);
			CueIt.RemoveCurrent();
		}
	}
}

/**
 * Updates the delay load delegate listeners based on the current load mode.
 * This function is responsible for resetting all listeners and adding new listeners based on the load mode configuration.
 * It removes existing listeners for gameplay tag loaded events, post garbage collection events, and post load map events.
 * Then, based on the value of GameplayCueManagerCvars::LoadMode, it adds new listeners to the respective delegates.
 * The listeners are added to the current instance of the UBaseGameplayCueManager class and the corresponding member functions are specified as the callback functions.
 *
 * @note This function is called when a gameplay tag is loaded and is used to update the listeners accordingly.
 */
void UBaseGameplayCueManager::UpdateDelayLoadDelegateListeners()
{
	// Reset all listeners
	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.RemoveAll(this);
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	switch (GameplayCueManagerCvars::LoadMode)
	{
	case EEditorLoadMode::LoadUpfront:
		return;
	case EEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		break;
	case EEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	// Add listeners based on the current mode
	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.AddUObject(this, &ThisClass::OnGameplayTagLoaded);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &ThisClass::HandlePostGarbageCollect);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

bool UBaseGameplayCueManager::ShouldDelayLoadGameplayCues() const
{
	return !IsRunningDedicatedServer();
}
