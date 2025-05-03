// Fill out your copyright notice in the Description page of Project Settings.

#include "ReplicationGraph/BaseReplicationGraphNode_AlwaysRelevant_ForConnection.h"

#include "GameFramework/PlayerState.h"
#include "ReplicationGraph/BaseReplicationGraph.h"
#include "ReplicationGraph/ReplicationGraphConsole.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseReplicationGraphNode_AlwaysRelevant_ForConnection)

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::ResetGameWorldState()
{
	ReplicationActorList.Reset();
	AlwaysRelevantStreamingLevelsNeedingReplication.Empty();
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
	UBaseReplicationGraph* Graph = CastChecked<UBaseReplicationGraph>(GetOuter());
	ReplicationActorList.Reset();

	ProcessViewers(Params);
	CleanupCachedRelevantActors(PastRelevantActorMap);
	ProcessAlwaysRelevantStreamingLevels(Params, Graph);

	Params.OutGatheredReplicationLists.AddReplicationActorList(ReplicationActorList);
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::ProcessViewers(const FConnectionGatherActorListParameters& Params)
{
	for (const FNetViewer& CurViewer : Params.Viewers)
	{
		ReplicationActorList.ConditionalAdd(CurViewer.InViewer);
		ReplicationActorList.ConditionalAdd(CurViewer.ViewTarget);

		if (const APlayerController* PC = Cast<APlayerController>(CurViewer.InViewer))
		{
			ProcessPlayerState(Params, PC);
			ProcessPawnAndViewTarget(Params, CurViewer, PC);
		}
	}
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::ProcessPlayerState(const FConnectionGatherActorListParameters& Params, const APlayerController* PC)
{
	const bool bReplicatePS = Params.ConnectionManager.ConnectionOrderNum % 2 == Params.ReplicationFrameNum % 2;
	if (!bReplicatePS || !PC->PlayerState)
		return;

	if (!bInitializedPlayerState)
	{
		bInitializedPlayerState = true;
		FConnectionReplicationActorInfo& ConnectionActorInfo = Params.ConnectionManager.ActorInfoMap.FindOrAdd(PC->PlayerState);
		ConnectionActorInfo.ReplicationPeriodFrame = 1;
	}
	ReplicationActorList.ConditionalAdd(PC->PlayerState);
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::ProcessPawnAndViewTarget(const FConnectionGatherActorListParameters& Params, const FNetViewer& CurViewer, const APlayerController* PC)
{
	FCachedAlwaysRelevantActorInfo& LastData = PastRelevantActorMap.FindOrAdd(CurViewer.Connection);

	if (const auto Pawn = PC->GetPawn())
	{
		UpdateCachedRelevantActor(Params, Pawn, LastData.LastViewer);
		if (Pawn != CurViewer.ViewTarget)
			ReplicationActorList.ConditionalAdd(Pawn);
	}

	if (const auto ViewTargetPawn = CurViewer.ViewTarget)
		UpdateCachedRelevantActor(Params, ViewTargetPawn, LastData.LastViewTarget);
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::ProcessAlwaysRelevantStreamingLevels(const FConnectionGatherActorListParameters& Params, UBaseReplicationGraph* LyraGraph)
{
	TMap<FName, FActorRepListRefView>& AlwaysRelevantStreamingLevelActors = LyraGraph->AlwaysRelevantStreamingLevelActors;

	for (int32 Idx = AlwaysRelevantStreamingLevelsNeedingReplication.Num() - 1; Idx >= 0; --Idx)
	{
		const FName& StreamingLevel = AlwaysRelevantStreamingLevelsNeedingReplication[Idx];
		if (FActorRepListRefView* Ptr = AlwaysRelevantStreamingLevelActors.Find(StreamingLevel))
		{
			ProcessStreamingLevelActors(Params, *Ptr, StreamingLevel, Idx);
			continue;
		}
		// If we don't have a RepList, remove the level from the list
		UE_CLOG(ReplicationGraphConsole::DisplayClientLevelStreaming > 0, LogRepGraph, Display,
		        TEXT("CLIENT STREAMING Removing %s from AlwaysRelevantStreamingLevelActors because FActorRepListRefView is null. %s "),
		        *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
		AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, EAllowShrinking::No);
	}
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::ProcessStreamingLevelActors(const FConnectionGatherActorListParameters& Params,
                                                                                         FActorRepListRefView& RepList,
                                                                                         const FName& StreamingLevel,
                                                                                         const int32 Idx)
{
	// Always relevant streaming level actors.
	FPerConnectionActorInfoMap& ConnectionActorInfoMap = Params.ConnectionManager.ActorInfoMap;

	if (RepList.Num() <= 0)
	{
		UE_LOG(LogRepGraph, Warning, TEXT("UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection - empty RepList %s"),
		       *Params.ConnectionManager.GetName());
		return;
	}
	// Check if all actors in the list are dormant on this connection
	auto bAllDormant = true;
	for (FActorRepListType Actor : RepList)
	{
		const FConnectionReplicationActorInfo& ConnectionActorInfo = ConnectionActorInfoMap.FindOrAdd(Actor);
		if (ConnectionActorInfo.bDormantOnConnection)
			continue;

		bAllDormant = false;
		break;
	}

	if (bAllDormant)
	{
		UE_CLOG(ReplicationGraphConsole::DisplayClientLevelStreaming > 0, LogRepGraph, Display,
		        TEXT("CLIENT STREAMING All AlwaysRelevant Actors Dormant on StreamingLevel %s for %s. Removing list."),
		        *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
		AlwaysRelevantStreamingLevelsNeedingReplication.RemoveAtSwap(Idx, EAllowShrinking::No);
		return;
	}
	// If we have at least one non dormant actor, add the list to the connection
	UE_CLOG(ReplicationGraphConsole::DisplayClientLevelStreaming > 0, LogRepGraph, Display,
	        TEXT("CLIENT STREAMING Adding always Actors on StreamingLevel %s for %s because it has at least one non dormant actor"),
	        *StreamingLevel.ToString(), *Params.ConnectionManager.GetName());
	Params.OutGatheredReplicationLists.AddReplicationActorList(RepList);
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd(const FName LevelName, UWorld* StreamingWorld)
{
	UE_CLOG(ReplicationGraphConsole::DisplayClientLevelStreaming > 0, LogRepGraph, Display, TEXT("CLIENT STREAMING ::OnClientLevelVisibilityAdd - %s"), *LevelName.ToString());
	AlwaysRelevantStreamingLevelsNeedingReplication.Add(LevelName);
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove(const FName LevelName)
{
	UE_CLOG(ReplicationGraphConsole::DisplayClientLevelStreaming > 0, LogRepGraph, Display, TEXT("CLIENT STREAMING ::OnClientLevelVisibilityRemove - %s"), *LevelName.ToString());
	AlwaysRelevantStreamingLevelsNeedingReplication.Remove(LevelName);
}

void UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::LogNode(FReplicationGraphDebugInfo& DebugInfo, const FString& NodeName) const
{
	DebugInfo.Log(NodeName);
	DebugInfo.PushIndent();
	LogActorRepList(DebugInfo, NodeName, ReplicationActorList);

	for (const FName& LevelName : AlwaysRelevantStreamingLevelsNeedingReplication)
	{
		UBaseReplicationGraph* LyraGraph = CastChecked<UBaseReplicationGraph>(GetOuter());
		if (const FActorRepListRefView* RepList = LyraGraph->AlwaysRelevantStreamingLevelActors.Find(LevelName))
		{
			LogActorRepList(DebugInfo, FString::Printf(TEXT("AlwaysRelevant StreamingLevel List: %s"),
			                                           *LevelName.ToString()), *RepList);
		}
	}

	DebugInfo.PopIndent();
}
