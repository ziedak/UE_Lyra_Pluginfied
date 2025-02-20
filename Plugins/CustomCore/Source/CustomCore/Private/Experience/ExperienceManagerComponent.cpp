#include "Experience/ExperienceManagerComponent.h"

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameFeaturesSubsystem.h"
#include "AssetManager/BaseAssetManager.h"
#include "Experience/ExperienceManagerSubsystem.h"
#include "Experience/DataAsset/ExperienceActionSet_DA.h"
#include "Experience/DataAsset/ExperienceDefinition_DA.h"
#include "Log/Log.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "Settings/LyraSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceManagerComponent)

namespace ConsoleVariables
{
	static float ExperienceLoadRandomDelayMin = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
		TEXT("lyra.chaos.ExperienceDelayLoad.MinSecs"),
		ExperienceLoadRandomDelayMin,
		TEXT(
			"This value (in seconds) will be added as a delay of load completion of the experience (along with the random value lyra.chaos.ExperienceDelayLoad.RandomSecs)"),
		ECVF_Default);

	static float ExperienceLoadRandomDelayRange = 0.0f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
		TEXT("lyra.chaos.ExperienceDelayLoad.RandomSecs"),
		ExperienceLoadRandomDelayRange,
		TEXT(
			"A random amount of time between 0 and this value (in seconds) will be added as a delay of load completion of the experience (along with the fixed value lyra.chaos.ExperienceDelayLoad.MinSecs)"),
		ECVF_Default);

	float GetExperienceLoadDelayDuration() { return FMath::Max(0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange); }
}


//@TODO: Async load the experience definition itself
//@TODO: Handle failures explicitly (go into a 'completed but failed' state rather than check()-ing)
//@TODO: Do the action phases at the appropriate times instead of all at once
//@TODO: Support deactivating an experience and do the unloading actions
//@TODO: Think about what deactivation/cleanup means for preloaded assets
//@TODO: Handle deactivating game features, right now we 'leak' them enabled
// (for a client moving from experience to experience we actually want to diff the requirements and only unload some, not unload everything for them to just be immediately reloaded)
//@TODO: Handle both built-in and URL-based plugins (search for colon?)

// Sets default values for this component's properties
UExperienceManagerComponent::UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) { SetIsReplicatedByDefault(true); }

const UExperienceDefinition_DA* UExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	check(LoadState == EExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

void UExperienceManagerComponent::SetCurrentExperience(const FPrimaryAssetId& ExperienceId)
{
	const UBaseAssetManager& AssetManager = UBaseAssetManager::Get();
	check(AssetManager.IsInitialized());

	const FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	const TSubclassOf<UExperienceDefinition_DA> AssetClass = Cast<UClass>(AssetPath.TryLoad());
	check(AssetClass);
	const UExperienceDefinition_DA* ExperienceDef = GetDefault<UExperienceDefinition_DA>(AssetClass);

	check(ExperienceDef != nullptr);
	check(CurrentExperience == nullptr);
	CurrentExperience = ExperienceDef;
	StartExperienceLoad();
}

void UExperienceManagerComponent::StartExperienceLoad()
{
	check(CurrentExperience != nullptr);
	check(LoadState == EExperienceLoadState::Unloaded);

	LOG_INFO(LogExperience, "EXPERIENCE: StartExperienceLoad(CurrentExperience = %s, %s)",
	         *CurrentExperience->GetPrimaryAssetId().ToString(),
	         *GetClientServerContext(this));

	LoadState = EExperienceLoadState::Loading;

	const TSet<FSoftObjectPath> RawAssetList;
	const TSet<FPrimaryAssetId> BundleAssetList = PrepareAssetLists();
	const TArray<FName> BundlesToLoad = PrepareBundlesToLoad();

	const TSharedPtr<FStreamableHandle> Handle = CreateStreamableHandle(BundleAssetList, RawAssetList,
	                                                                    BundlesToLoad);

	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(
		this, &ThisClass::OnExperienceLoadComplete);

	// Assets were already loaded, call the delegate now
	if (!Handle.IsValid() || Handle->HasLoadCompleted()) FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	else
	{
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);
		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate](){ auto _ = OnAssetsLoadedDelegate.ExecuteIfBound(); }));
	}

	PreloadAssets(BundlesToLoad);
}


TSet<FPrimaryAssetId> UExperienceManagerComponent::PrepareAssetLists() const
{
	TSet<FPrimaryAssetId> BundleAssetListIds;
	BundleAssetListIds.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UExperienceActionSet_DA>& ActionSet : CurrentExperience->ExperienceActionSets)
	{
		if (!ActionSet) continue;
		BundleAssetListIds.Add(ActionSet->GetPrimaryAssetId());
	}

	return BundleAssetListIds;
}

TArray<FName> UExperienceManagerComponent::PrepareBundlesToLoad() const
{
	TArray<FName> BundlesToLoad;
	BundlesToLoad.Add("Equipped");

	//@TODO: Centralize this client/server stuff into the LyraAssetManager
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || OwnerNetMode != NM_DedicatedServer;
	const bool bLoadServer = GIsEditor || OwnerNetMode != NM_Client;

	if (bLoadClient) BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	if (bLoadServer) BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);

	return BundlesToLoad;
}

TSharedPtr<FStreamableHandle> UExperienceManagerComponent::CreateStreamableHandle(const TSet<FPrimaryAssetId>& BundleAssetList,
                                                                                  const TSet<FSoftObjectPath>& RawAssetList,
                                                                                  const TArray<FName>& BundlesToLoad) const
{
	UAssetManager& AssetManager = UAssetManager::Get();
	check(AssetManager.IsInitialized());

	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array(),
		                                                                  BundlesToLoad,
		                                                                  {},
		                                                                  false,
		                                                                  FStreamableDelegate(),
		                                                                  FStreamableManager::AsyncLoadHighPriority);
	}

	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(),
		                                           FStreamableDelegate(),
		                                           FStreamableManager::AsyncLoadHighPriority,
		                                           TEXT("StartExperienceLoad()"));
	}

	// If both async loads are running, combine them
	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		return AssetManager.GetStreamableManager()
		                   .CreateCombinedHandle({BundleLoadHandle, RawLoadHandle});
	}

	return BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
}

void UExperienceManagerComponent::PreloadAssets(const TArray<FName>& BundlesToLoad) const
{
	// This set of assets gets preloaded, but we don't block the start of the experience based on it
	UAssetManager& AssetManager = UAssetManager::Get();
	check(AssetManager.IsInitialized());

	const TSet<FPrimaryAssetId> PreloadAssetList;
	//@TODO: Determine assets to preload (but not blocking-ly)
	if (PreloadAssetList.Num() <= 0) return;

	AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(),
	                                               BundlesToLoad,
	                                               {});
}


void UExperienceManagerComponent::OnExperienceLoadComplete()
{
	check(LoadState == EExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	ULOG_INFO(LogExperience, "EXPERIENCE: OnExperienceLoadComplete(CurrentExperience =  %s, %s)",
	          *CurrentExperience->GetPrimaryAssetId().ToString(),
	          *GetClientServerContext(this));

	// find the URLs for our GameFeaturePlugins - filtering out dupes and ones that don't have a valid mapping
	GameFeaturePluginURLs.Reset();

	GameFeaturePluginURLs.Append(
		CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnableList));

	for (const TObjectPtr<UExperienceActionSet_DA>& ActionSet : CurrentExperience->ExperienceActionSets)
	{
		if (!ActionSet) continue;

		GameFeaturePluginURLs.Append(CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnableList));
	}

	// Load and activate the features
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading <= 0)
	{
		OnExperienceFullLoadCompleted();
		return;
	}

	LoadState = EExperienceLoadState::LoadingGameFeatures;
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		if (PluginURL.IsEmpty()) continue;

		UExperienceManagerSubsystem::NotifyOfPluginActivation(PluginURL);
		UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL,
		                                                               FGameFeaturePluginLoadComplete::CreateUObject(this,
		                                                                                                             &ThisClass::OnGameFeaturePluginLoadComplete)
		);
	}
}

TArray<FString> UExperienceManagerComponent::CollectGameFeaturePluginURLs(
	const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList) const
{
	// find the URLs for our GameFeaturePlugins - filtering out dupes and ones that don't have a valid mapping
	TArray<FString> PluginURLs;
	for (const FString& PluginName : FeaturePluginList)
	{
		FString PluginURL;
		if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, /*out*/ PluginURL))
		{
			PluginURLs.AddUnique(PluginURL);
			continue;
		}

		ULOG_INFO(LogExperience,
		          "OnExperienceLoadComplete failed to find plugin URL from PluginName %s for experience %s - fix data, ignoring for this run"
		          , *PluginName,
		          *Context->GetPrimaryAssetId().ToString());
	}

	// Add in our extra plugin
	// if (!CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent.IsEmpty())
	// {
	// 	FString PluginURL;
	// 	if (UGameFeaturesSubsystem::Get().GetPluginURLByName(CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent, PluginURL))
	// 	{
	// 		PluginURLs.AddUnique(PluginURL);
	// 	}
	// }

	return PluginURLs;
}

void UExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	// decrement the number of plugins that are loading
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading == 0) OnExperienceFullLoadCompleted();
}


void UExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
	check(LoadState != EExperienceLoadState::Loaded);

	// Insert a random delay for testing (if configured)
	const float DelaySecs = ConsoleVariables::GetExperienceLoadDelayDuration();
	InsertRandomDelayForTesting(DelaySecs);

	LoadState = EExperienceLoadState::ExecutingActions;
	ExecuteActions();

	LoadState = EExperienceLoadState::Loaded;
	BroadcastExperienceLoaded();

	// Apply any necessary scalability settings
	ApplyScalabilitySettings();
}

void UExperienceManagerComponent::InsertRandomDelayForTesting(const float DelaySecs)
{
	if (LoadState == EExperienceLoadState::LoadingChaosTestingDelay || DelaySecs <= 0.0f) return;

	FTimerHandle DummyHandle;

	LoadState = EExperienceLoadState::LoadingChaosTestingDelay;
	GetWorld()->GetTimerManager().SetTimer(DummyHandle, this, &ThisClass::OnExperienceFullLoadCompleted,
	                                       DelaySecs, /*bLooping=*/ false);
}

void UExperienceManagerComponent::ExecuteActions() const
{
	FGameFeatureActivatingContext Context;

	const auto ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext) Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);

	ActivateListOfActions(CurrentExperience->GameFeatureActions, Context);
	for (const TObjectPtr<UExperienceActionSet_DA>& ActionSet : CurrentExperience->ExperienceActionSets)
	{
		if (!ActionSet) continue;

		ActivateListOfActions(ActionSet->GameFeatureActions, Context);
	}
}

void UExperienceManagerComponent::ActivateListOfActions(const TArray<UGameFeatureAction*>& ActionList,
                                                        FGameFeatureActivatingContext& Context) const
{
	for (UGameFeatureAction* Action : ActionList)
	{
		if (!Action) continue;

		//@TODO: The fact that these don't take a world are potentially problematic in client-server PIE
		// The current behavior matches systems like gameplay tags where loading and registering apply to the entire process,
		// but actually applying the results to actors is restricted to a specific world
		Action->OnGameFeatureRegistering();
		Action->OnGameFeatureLoading();
		Action->OnGameFeatureActivating(Context);
	}
}

void UExperienceManagerComponent::BroadcastExperienceLoaded()
{
	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();
}

void UExperienceManagerComponent::ApplyScalabilitySettings()
{
#if !UE_SERVER
	// Apply any necessary scalability settings
	if (const auto SettingsLocal = ULyraSettingsLocal::Get()) SettingsLocal->OnExperienceLoaded();
#endif
}


void UExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
}

void UExperienceManagerComponent::OnRep_CurrentExperience() { StartExperienceLoad(); }

#pragma region ExperienceLoadedPriority
void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(
	FOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
		return;
	}
	OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
}

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
		return;
	}
	OnExperienceLoaded.Add(MoveTemp(Delegate));
}

void UExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(
	FOnExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
		return;
	}
	OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
}
#pragma endregion ExperienceLoadedPriority

#pragma region Deactivation
void UExperienceManagerComponent::OnActionDeactivationCompleted()
{
	check(IsInGameThread());
	++NumObservedPausers;

	if (NumObservedPausers == NumExpectedPausers) OnAllActionsDeactivated();
}

void UExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DeactivateLoadedFeatures();
	HandlePartiallyLoadedState();
}

void UExperienceManagerComponent::DeactivateLoadedFeatures()
{
	// deactivate any features this experience loaded
	//@TODO: This should be handled FILO as well
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		if (UExperienceManagerSubsystem::RequestToDeactivatePlugin(PluginURL)) UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
	}
}

void UExperienceManagerComponent::HandlePartiallyLoadedState()
{
	//@TODO: Ensure proper handling of a partially-loaded state too
	if (LoadState != EExperienceLoadState::Loaded) return;

	LoadState = EExperienceLoadState::Deactivating;
	NumExpectedPausers = INDEX_NONE;
	NumObservedPausers = 0;

	FGameFeatureDeactivatingContext Context(
		TEXT(""), [this](FStringView){ this->OnActionDeactivationCompleted(); });

	if (const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld())) Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);

	DeactivateListOfActions(CurrentExperience->GameFeatureActions, Context);

	for (const TObjectPtr<UExperienceActionSet_DA>& ActionSet : CurrentExperience->ExperienceActionSets)
	{
		if (!ActionSet) continue;

		DeactivateListOfActions(ActionSet->GameFeatureActions, Context);
	}

	NumExpectedPausers = Context.GetNumPausers();

	if (NumExpectedPausers > 0)
		ULOG_ERROR(LogExperience,
	           "Actions that have asynchronous deactivation aren't fully supported yet in experiences");

	if (NumExpectedPausers == NumObservedPausers) OnAllActionsDeactivated();
}


void UExperienceManagerComponent::DeactivateListOfActions(const TArray<UGameFeatureAction*>& ActionList,
                                                          FGameFeatureDeactivatingContext& Context) const
{
	for (UGameFeatureAction* Action : ActionList)
	{
		if (!Action) continue;

		Action->OnGameFeatureDeactivating(Context);
		Action->OnGameFeatureUnregistering();
	}
}

void UExperienceManagerComponent::OnAllActionsDeactivated()
{
	//@TODO: We actually only deactivated and didn't fully unload...
	LoadState = EExperienceLoadState::Unloaded;
	CurrentExperience = nullptr;
	//@TODO:	GEngine->ForceGarbageCollection(true);
}
#pragma endregion Deactivation

bool UExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (LoadState == EExperienceLoadState::Loaded) return false;

	OutReason = TEXT("Experience still loading");
	return true;
}