// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RepGraphActorClassSettings.h"
#include "ReplicationGraph.h"
#include "BaseReplicationGraph.generated.h"

class AGameplayDebuggerCategoryReplicator;
// This is the main enum we use to route actors to the right replication node. Each class maps to one enum.

UCLASS(transient, config=Engine)
class UBaseReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	UBaseReplicationGraph();

	virtual void ResetGameWorldState() override;

	virtual void InitGlobalActorClassSettings() override;
	void SetupLazyInitFunction();
	void LogLazyInitClasses(UClass* Class, const FClassReplicationInfo& ClassInfo, bool bHandled);
	void SetupClassNodeMappings();
	void SetupAllReplicatedClasses();
	void SetupCharacterClassReplicationInfo();
	void ValidateSharedRepMovement();
	void SetupRPCMulticastOpenChannelForClass();
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

	UPROPERTY()
	TArray<TObjectPtr<UClass>> AlwaysRelevantClasses;

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	TMap<FName, FActorRepListRefView> AlwaysRelevantStreamingLevelActors;

#if WITH_GAMEPLAY_DEBUGGER
	void OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner);
#endif

	void PrintRepNodePolicies();

private:
	void AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping);
	void RegisterClassRepNodeMapping(UClass* Class);
	EClassRepNodeMapping GetClassNodeMapping(UClass* Class) const;

	void RegisterClassReplicationInfo(UClass* ReplicatedClass);
	bool ConditionalInitClassReplicationInfo(UClass* ReplicatedClass, FClassReplicationInfo& ClassInfo);
	void InitClassReplicationInfo(FClassReplicationInfo& Info, const UClass* Class, bool Spatialize) const;

	EClassRepNodeMapping GetMappingPolicy(UClass* Class);

	bool IsSpatialized(const EClassRepNodeMapping Mapping) const
	{
		return Mapping >= EClassRepNodeMapping::Spatialize_Static;
	}

	TClassMap<EClassRepNodeMapping> ClassRepNodePolicies;

	/** Classes that had their replication settings explicitly set by code in ULyraReplicationGraph::InitGlobalActorClassSettings */
	TArray<UClass*> ExplicitlySetClasses;
};
