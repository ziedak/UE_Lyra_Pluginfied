// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "LoadingProcessInterface.h"
#include "Components/GameStateComponent.h"
#include "ExperienceManagerComponent.generated.h"

struct FStreamableHandle;
struct FGameFeatureActivatingContext;
struct FGameFeatureDeactivatingContext;
class UGameFeatureAction;
class UExperienceDefinition_DA;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExperienceLoaded, const UExperienceDefinition_DA* /*Experience*/);

enum class EExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

namespace UE::GameFeatures
{
	struct FResult;
}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CUSTOMCORE_API UExperienceManagerComponent : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	UExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	//~ILoadingProcessInterface interface
	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;
	//~End of ILoadingProcessInterface

	// Tries to set the current experience, either a UI or gameplay one
	void SetCurrentExperience(const FPrimaryAssetId& ExperienceId);

	// Ensures the delegate is called once the experience has been loaded,
	// before others are called.
	// However, if the experience has already loaded, calls the delegate immediately.
	void CallOrRegister_OnExperienceLoaded_HighPriority(FOnExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate&& Delegate);

	// Ensures the delegate is called once the experience has been loaded
	// If the experience has already loaded, calls the delegate immediately
	void CallOrRegister_OnExperienceLoaded_LowPriority(FOnExperienceLoaded::FDelegate&& Delegate);

	// This returns the current experience if it is fully loaded, asserting otherwise
	// (i.e., if you called it too soon)
	const UExperienceDefinition_DA* GetCurrentExperienceChecked() const;

	// Returns true if the experience is fully loaded
	FORCEINLINE bool IsExperienceLoaded() const { return LoadState == EExperienceLoadState::Loaded && CurrentExperience; }

private:
	TSharedPtr<FStreamableHandle> CreateStreamableHandle(const TSet<FPrimaryAssetId>& BundleAssetList,
	                                                     const TSet<FSoftObjectPath>& RawAssetList,
	                                                     const TArray<FName>& BundlesToLoad) const;
	TSet<FPrimaryAssetId> PrepareAssetLists() const;
	TArray<FName> PrepareBundlesToLoad() const;
	void PreloadAssets(const TArray<FName>& BundlesToLoad) const;
	void StartExperienceLoad();
	void OnExperienceLoadComplete();
	TArray<FString> CollectGameFeaturePluginURLs(const UPrimaryDataAsset* Context,
	                                             const TArray<FString>& FeaturePluginList) const;
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void ExecuteActions() const;
	void BroadcastExperienceLoaded();
	void ApplyScalabilitySettings();
	void OnExperienceFullLoadCompleted();
	void InsertRandomDelayForTesting(float DelaySecs);

	void OnActionDeactivationCompleted();
	void OnAllActionsDeactivated();
	void ActivateListOfActions(const TArray<UGameFeatureAction*>& ActionList, FGameFeatureActivatingContext& Context) const;

	void DeactivateListOfActions(const TArray<UGameFeatureAction*>& ActionList, FGameFeatureDeactivatingContext& Context) const;
	void DeactivateLoadedFeatures();
	void HandlePartiallyLoadedState();

	UFUNCTION()
	void OnRep_CurrentExperience();
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const UExperienceDefinition_DA> CurrentExperience;

	EExperienceLoadState LoadState = EExperienceLoadState::Unloaded;

	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;

	int32 NumObservedPausers = 0;
	int32 NumExpectedPausers = 0;

	/**
	 * Delegate called when the experience has finished loading just before others
	 * (e.g., subsystems that set up for regular gameplay)
	 */
	FOnExperienceLoaded OnExperienceLoaded_HighPriority;

	/** Delegate called when the experience has finished loading */
	FOnExperienceLoaded OnExperienceLoaded;

	/** Delegate called when the experience has finished loading */
	FOnExperienceLoaded OnExperienceLoaded_LowPriority;
};