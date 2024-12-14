// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/GameFeaturePolicy.h"
#include "GameFeatures/GameFeature_AddGameplayCuePaths.h"
#include "GameFeatures/GameFeature_HotfixManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeaturePolicy)

UGameFeaturePolicy::UGameFeaturePolicy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGameFeaturePolicy& UGameFeaturePolicy::Get()
{
	return UGameFeaturesSubsystem::Get().GetPolicy<UGameFeaturePolicy>();
}

void UGameFeaturePolicy::InitGameFeatureManager()
{
	Observers.Add(NewObject<UGameFeature_HotfixManager>());
	Observers.Add(NewObject<UGameFeature_AddGameplayCuePaths>());

	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		Subsystem.AddObserver(Observer);
	}

	Super::InitGameFeatureManager();
}

void UGameFeaturePolicy::ShutdownGameFeatureManager()
{
	Super::ShutdownGameFeatureManager();

	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
	for (UObject* Observer : Observers)
	{
		Subsystem.RemoveObserver(Observer);
	}
	Observers.Empty();
}

TArray<FPrimaryAssetId> UGameFeaturePolicy::GetPreloadAssetListForGameFeature(
	const UGameFeatureData* GameFeatureToLoad, bool bIncludeLoadedAssets) const
{
	return Super::GetPreloadAssetListForGameFeature(GameFeatureToLoad, bIncludeLoadedAssets);
}

const TArray<FName> UGameFeaturePolicy::GetPreloadBundleStateForGameFeature() const
{
	return Super::GetPreloadBundleStateForGameFeature();
}

void UGameFeaturePolicy::GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const
{
	// Editor will load both, this can cause hitching as the bundles are set to not preload in editor
	bLoadClientData = !IsRunningDedicatedServer();
	bLoadServerData = !IsRunningClientOnly();
}

bool UGameFeaturePolicy::IsPluginAllowed(const FString& PluginURL) const
{
	return Super::IsPluginAllowed(PluginURL);
}
