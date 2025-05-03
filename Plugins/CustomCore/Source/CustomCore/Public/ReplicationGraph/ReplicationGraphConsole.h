#pragma once
#include "BaseReplicationGraph.h"
#include "Development/ReplicationGraphSettings.h"
#include "Log/Log.h"

namespace ReplicationGraphConsole
{
	inline auto DestructionInfoMaxDist = 30000.f;
	static FAutoConsoleVariableRef CVarLyraRepGraphDestructMaxDist(TEXT("RepGraph.DestructInfo.MaxDist"),
	                                                               DestructionInfoMaxDist,
	                                                               TEXT("Max distance (not squared) to rep destruct infos at"),
	                                                               ECVF_Default);

	inline auto DisplayClientLevelStreaming = 0;
	static FAutoConsoleVariableRef CVarLyraRepGraphDisplayClientLevelStreaming(TEXT("RepGraph.DisplayClientLevelStreaming"),
	                                                                           DisplayClientLevelStreaming,
	                                                                           TEXT(""),
	                                                                           ECVF_Default);

	inline auto CellSize = 10000.f;
	static FAutoConsoleVariableRef CVarLyraRepGraphCellSize(TEXT("RepGraph.CellSize"),
	                                                        CellSize,
	                                                        TEXT(""),
	                                                        ECVF_Default);

	// Essentially "Min X" for replication. This is just an initial value. The system will reset itself if actors appears outside of this.
	inline auto SpatialBiasX = -150000.f;
	static FAutoConsoleVariableRef CVarLyraRepGraphSpatialBiasX(TEXT("RepGraph.SpatialBiasX"),
	                                                            SpatialBiasX,
	                                                            TEXT(""),
	                                                            ECVF_Default);

	// Essentially "Min Y" for replication. This is just an initial value. The system will reset itself if actors appears outside of this.
	inline auto SpatialBiasY = -200000.f;
	static FAutoConsoleVariableRef CVarLyraRepSpatialBiasY(TEXT("RepGraph.SpatialBiasY"),
	                                                       SpatialBiasY,
	                                                       TEXT(""),
	                                                       ECVF_Default);

	// How many buckets to spread dynamic,
	// spatialized actors across. High number = more buckets = smaller effective replication frequency. This happens before individual actors do their own NetUpdateFrequency check.
	inline auto DynamicActorFrequencyBuckets = 3;
	static FAutoConsoleVariableRef CVarLyraRepDynamicActorFrequencyBuckets(TEXT("RepGraph.DynamicActorFrequencyBuckets"),
	                                                                       DynamicActorFrequencyBuckets,
	                                                                       TEXT(""),
	                                                                       ECVF_Default);

	inline auto DisableSpatialRebuilds = 1;
	static FAutoConsoleVariableRef CVarLyraRepDisableSpatialRebuilds(TEXT("RepGraph.DisableSpatialRebuilds"),
	                                                                 DisableSpatialRebuilds,
	                                                                 TEXT(""),
	                                                                 ECVF_Default);

	inline auto LogLazyInitClasses = 0;
	static FAutoConsoleVariableRef CVarLyraRepLogLazyInitClasses(TEXT("RepGraph.LogLazyInitClasses"),
	                                                             LogLazyInitClasses,
	                                                             TEXT(""),
	                                                             ECVF_Default);

	// How much bandwidth to use for FastShared movement updates. This is counted independently of the NetDriver's target bandwidth.
	inline auto TargetKBytesSecFastSharedPath = 10;
	static FAutoConsoleVariableRef CVarLyraRepTargetKBytesSecFastSharedPath(TEXT("RepGraph.TargetKBytesSecFastSharedPath"),
	                                                                        TargetKBytesSecFastSharedPath,
	                                                                        TEXT(""),
	                                                                        ECVF_Default);

	inline auto FastSharedPathCullDistPct = 0.80f;
	static FAutoConsoleVariableRef CVarLyraRepFastSharedPathCullDistPct(TEXT("RepGraph.FastSharedPathCullDistPct"),
	                                                                    FastSharedPathCullDistPct,
	                                                                    TEXT(""),
	                                                                    ECVF_Default);

	inline auto EnableFastSharedPath = 1;
	static FAutoConsoleVariableRef CVarLyraRepEnableFastSharedPath(TEXT("RepGraph.EnableFastSharedPath"),
	                                                               EnableFastSharedPath,
	                                                               TEXT(""),
	                                                               ECVF_Default);

	inline UReplicationDriver* ConditionalCreateReplicationDriver(const UNetDriver* ForNetDriver,
	                                                              const UWorld* World)
	{
		// Only create for GameNetDriver
		if (!World || !ForNetDriver || ForNetDriver->NetDriverName != NAME_GameNetDriver)
			return nullptr;

		const auto LyraRepGraphSettings = GetDefault<UReplicationGraphSettings>();

		// Enable/Disable via developer settings
		if (LyraRepGraphSettings && LyraRepGraphSettings->bDisableReplicationGraph)
		{
			UE_LOG(LogRepGraph, Display, TEXT("Replication graph is disabled via LyraReplicationGraphSettings."));
			return nullptr;
		}

		UE_LOG(LogRepGraph, Display,
		       TEXT("Replication graph is enabled for %s in world %s."),
		       *GetNameSafe(ForNetDriver),
		       *GetPathNameSafe(World));

		TSubclassOf<UBaseReplicationGraph> GraphClass = LyraRepGraphSettings->DefaultReplicationGraphClass.TryLoadClass<UBaseReplicationGraph>();
		if (!GraphClass.Get())
			GraphClass = UBaseReplicationGraph::StaticClass();

		UBaseReplicationGraph* LyraReplicationGraph = NewObject<UBaseReplicationGraph>(GetTransientPackage(),
		                                                                               GraphClass.Get());
		return LyraReplicationGraph;
	}


	inline FAutoConsoleCommandWithWorldAndArgs LyraPrintRepNodePoliciesCmd(
		TEXT("RepGraph.PrintRouting"),TEXT("Prints how actor classes are routed to RepGraph nodes"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			for (TObjectIterator<UBaseReplicationGraph> It; It; ++It)
			{
				It->PrintRepNodePolicies();
			}
		})
	);


	inline FAutoConsoleCommandWithWorldAndArgs ChangeFrequencyBucketsCmd(
		TEXT("RepGraph.FrequencyBuckets"), TEXT("Resets frequency bucket count."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			auto Buckets = 1;
			if (Args.Num() > 0)
				LexTryParseString<int32>(Buckets, *Args[0]);

			UE_LOG(LogRepGraph, Display, TEXT("Setting Frequency Buckets to %d"), Buckets);

			for (TObjectIterator<UReplicationGraphNode_ActorListFrequencyBuckets> It; It; ++It)
			{
				UReplicationGraphNode_ActorListFrequencyBuckets* Node = *It;
				Node->SetNonStreamingCollectionSize(Buckets);
			}
		}));
};
