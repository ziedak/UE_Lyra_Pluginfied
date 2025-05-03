// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseReplicationGraph.h"
#include "ReplicationGraph.h"
#include "BaseReplicationGraphNode_AlwaysRelevant_ForConnection.generated.h"

class AGameplayDebuggerCategoryReplicator;
/**
 * 
 */
UCLASS()
class CUSTOMCORE_API UBaseReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode_AlwaysRelevant_ForConnection
{
	GENERATED_BODY()

public:
	virtual void NotifyAddNetworkActor(const FNewReplicatedActorInfo& Actor) override
	{
	}

	virtual bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound = true) override { return false; }

	virtual void NotifyResetAllNetworkActors() override
	{
	}

	virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;
	void ProcessViewers(const FConnectionGatherActorListParameters& Params);
	void ProcessPlayerState(const FConnectionGatherActorListParameters& Params, const APlayerController* PC);
	void ProcessPawnAndViewTarget(const FConnectionGatherActorListParameters& Params, const FNetViewer& CurViewer, const APlayerController* PC);
	void ProcessAlwaysRelevantStreamingLevels(const FConnectionGatherActorListParameters& Params, UBaseReplicationGraph* LyraGraph);
	void ProcessStreamingLevelActors(const FConnectionGatherActorListParameters& Params, FActorRepListRefView& RepList, const FName& StreamingLevel, int32 Idx);

	virtual void LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const override;

	void OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld);
	void OnClientLevelVisibilityRemove(FName LevelName);

	void ResetGameWorldState();

#if WITH_GAMEPLAY_DEBUGGER
	TWeakObjectPtr<AGameplayDebuggerCategoryReplicator> GameplayDebugger = nullptr;
#endif

private:
	TArray<FName, TInlineAllocator<64>> AlwaysRelevantStreamingLevelsNeedingReplication;

	bool bInitializedPlayerState = false;
};
