// Fill out your copyright notice in the Description page of Project Settings.


#include "ReplicationGraph/BaseReplicationGraph.h"

#include "GameplayDebuggerCategoryReplicator.h"
#include "Development/ReplicationGraphSettings.h"
#include "Engine/ServerStatReplicator.h"
#include "GameFramework/Character.h"
#include "Interface/ISharedReplication.h"
#include "Log/Log.h"
#include "ReplicationGraph/BaseReplicationGraphNode_AlwaysRelevant_ForConnection.h"
#include "ReplicationGraph/RepGraphActorClassSettings.h"
#include "ReplicationGraph/ReplicationGraphConsole.h"
#include "ReplicationGraph/ReplicationGraphNode_PlayerStateFrequencyLimiter.h"
#include "ReplicationGraph/SharedRepMovement.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseReplicationGraph)

UBaseReplicationGraph::UBaseReplicationGraph()
{
	if (CreateReplicationDriverDelegate().IsBound())
		return;

	CreateReplicationDriverDelegate().BindLambda(
		[](const UNetDriver* ForNetDriver, const FURL& URL, const UWorld* World) -> UReplicationDriver*
		{
			return ReplicationGraphConsole::ConditionalCreateReplicationDriver(ForNetDriver, World);
		});
}

void UBaseReplicationGraph::ResetGameWorldState()
{
	Super::ResetGameWorldState();

	AlwaysRelevantStreamingLevelActors.Empty();

	for (const auto ConnManager : Connections)
	{
		for (const auto ConnectionNode : ConnManager->GetConnectionGraphNodes())
		{
			if (const auto AlwaysRelevantConnectionNode = Cast<UBaseReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
				AlwaysRelevantConnectionNode->ResetGameWorldState();
		}
	}

	for (const auto ConnManager : PendingConnections)
	{
		for (const auto ConnectionNode : ConnManager->GetConnectionGraphNodes())
		{
			if (const auto AlwaysRelevantConnectionNode = Cast<UBaseReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
				AlwaysRelevantConnectionNode->ResetGameWorldState();
		}
	}
}

EClassRepNodeMapping UBaseReplicationGraph::GetClassNodeMapping(UClass* Class) const
{
	if (!Class)
		return EClassRepNodeMapping::NotRouted;

	if (const EClassRepNodeMapping* Ptr = ClassRepNodePolicies.FindWithoutClassRecursion(Class))
		return *Ptr;

	const AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
	if (!ActorCDO || !ActorCDO->GetIsReplicated())
		return EClassRepNodeMapping::NotRouted;

	auto ShouldSpatialize = [](const AActor* CDO)
	{
		return CDO->GetIsReplicated() && (!(CDO->bAlwaysRelevant || CDO->bOnlyRelevantToOwner || CDO->bNetUseOwnerRelevancy));
	};

	// auto GetLegacyDebugStr = [](const AActor* CDO)
	// {
	// 	return FString::Printf(TEXT("%s [%d/%d/%d]"), *CDO->GetClass()->GetName(), CDO->bAlwaysRelevant, CDO->bOnlyRelevantToOwner, CDO->bNetUseOwnerRelevancy);
	// };

	// Only handle this class if it differs from its super. There is no need to put every child class explicitly in the graph class mapping
	UClass* SuperClass = Class->GetSuperClass();
	if (const AActor* SuperCDO = Cast<AActor>(SuperClass->GetDefaultObject()))
	{
		if (SuperCDO->GetIsReplicated() == ActorCDO->GetIsReplicated()
			&& SuperCDO->bAlwaysRelevant == ActorCDO->bAlwaysRelevant
			&& SuperCDO->bOnlyRelevantToOwner == ActorCDO->bOnlyRelevantToOwner
			&& SuperCDO->bNetUseOwnerRelevancy == ActorCDO->bNetUseOwnerRelevancy
		)
			return GetClassNodeMapping(SuperClass);
	}

	if (ShouldSpatialize(ActorCDO))
		return EClassRepNodeMapping::Spatialize_Dynamic;

	if (ActorCDO->bAlwaysRelevant && !ActorCDO->bOnlyRelevantToOwner)
		return EClassRepNodeMapping::RelevantAllConnections;

	return EClassRepNodeMapping::NotRouted;
}

void UBaseReplicationGraph::RegisterClassRepNodeMapping(UClass* Class)
{
	ClassRepNodePolicies.Set(Class, GetClassNodeMapping(Class));
}

void UBaseReplicationGraph::InitClassReplicationInfo(FClassReplicationInfo& Info, const UClass* Class, const bool Spatialize) const
{
	const AActor* CDO = Class->GetDefaultObject<AActor>();
	if (Spatialize)
	{
		Info.SetCullDistanceSquared(CDO->GetNetCullDistanceSquared());
		UE_LOG(LogRepGraph, Log, TEXT("Setting cull distance for %s to %f (%f)"), *Class->GetName(), Info.GetCullDistanceSquared(), Info.GetCullDistance());
	}

	Info.ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(CDO->GetNetUpdateFrequency());

	const UClass* NativeClass = Class;
	while (!NativeClass->IsNative() && NativeClass->GetSuperClass() && NativeClass->GetSuperClass() != AActor::StaticClass())
	{
		NativeClass = NativeClass->GetSuperClass();
	}

	UE_LOG(LogRepGraph, Log, TEXT("Setting replication period for %s (%s) to %d frames (%.2f)"),
	       *Class->GetName(),
	       *NativeClass->GetName(),
	       Info.ReplicationPeriodFrame,
	       CDO->GetNetUpdateFrequency());
}

bool UBaseReplicationGraph::ConditionalInitClassReplicationInfo(UClass* ReplicatedClass, FClassReplicationInfo& ClassInfo)
{
	const auto ReplicatedChildClass = ExplicitlySetClasses.FindByPredicate([&](const UClass* SetClass)
	{
		return ReplicatedClass->IsChildOf(SetClass);
	});

	if (ReplicatedChildClass)
		return false;

	const bool ClassIsSpatialized = IsSpatialized(ClassRepNodePolicies.GetChecked(ReplicatedClass));
	InitClassReplicationInfo(ClassInfo, ReplicatedClass, ClassIsSpatialized);
	return true;
}

void UBaseReplicationGraph::AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping)
{
	if (IsSpatialized(Mapping) && Class->GetDefaultObject<AActor>()->bAlwaysRelevant)
	{
		UE_LOG(LogRepGraph, Warning, TEXT("Replicated Class %s is AlwaysRelevant but is initialized into a spatialized node (%s)"),
		       *Class->GetName(),
		       *StaticEnum<EClassRepNodeMapping>()->GetNameStringByValue((int64)Mapping));
	}

	ClassRepNodePolicies.Set(Class, Mapping);
}

void UBaseReplicationGraph::RegisterClassReplicationInfo(UClass* ReplicatedClass)
{
	FClassReplicationInfo ClassInfo;
	if (!ConditionalInitClassReplicationInfo(ReplicatedClass, ClassInfo))
		return;

	GlobalActorReplicationInfoMap.SetClassInfo(ReplicatedClass, ClassInfo);
	UE_LOG(LogRepGraph, Log, TEXT("Setting %s - %.2f"), *GetNameSafe(ReplicatedClass), ClassInfo.GetCullDistance());
}

void UBaseReplicationGraph::InitGlobalActorClassSettings()
{
	SetupLazyInitFunction();
	SetupClassNodeMappings();
	SetupAllReplicatedClasses();
	SetupCharacterClassReplicationInfo();
	SetupRPCMulticastOpenChannelForClass();
}

void UBaseReplicationGraph::SetupLazyInitFunction()
{
	GlobalActorReplicationInfoMap.SetInitClassInfoFunc(
		[this](UClass* Class, FClassReplicationInfo& ClassInfo)
		{
			RegisterClassRepNodeMapping(Class);
			const bool bHandled = ConditionalInitClassReplicationInfo(Class, ClassInfo);
			LogLazyInitClasses(Class, ClassInfo, bHandled);
			return bHandled;
		});
}

void UBaseReplicationGraph::LogLazyInitClasses(UClass* Class, const FClassReplicationInfo& ClassInfo, const bool bHandled)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (ReplicationGraphConsole::LogLazyInitClasses == 0)
		return;
	if (bHandled)
	{
		EClassRepNodeMapping Mapping = ClassRepNodePolicies.GetChecked(Class);
		UE_LOG(LogRepGraph, Warning, TEXT("%s was Lazy Initialized. (Parent: %s) %d."), *GetNameSafe(Class), *GetNameSafe(Class->GetSuperClass()), (int32)Mapping);

		const FClassReplicationInfo& ParentRepInfo = GlobalActorReplicationInfoMap.GetClassInfo(Class->GetSuperClass());
		if (ClassInfo.BuildDebugStringDelta() != ParentRepInfo.BuildDebugStringDelta())
		{
			UE_LOG(LogRepGraph, Warning, TEXT("Differences Found!"));
			UE_LOG(LogRepGraph, Warning, TEXT("  Parent: %s"), *ParentRepInfo.BuildDebugStringDelta());
			UE_LOG(LogRepGraph, Warning, TEXT("  Class : %s"), *ClassInfo.BuildDebugStringDelta());
		}
		return;
	}
	// If we get here, the class was not handled by the lazy init function
	UE_LOG(LogRepGraph, Warning, TEXT("%s skipped Lazy Initialization because it does not differ from its parent. (Parent: %s)"), *GetNameSafe(Class),
	       *GetNameSafe(Class->GetSuperClass()));
#endif
}

void UBaseReplicationGraph::SetupClassNodeMappings()
{
	ClassRepNodePolicies.InitNewElement = [this](UClass* Class, EClassRepNodeMapping& NodeMapping)
	{
		NodeMapping = GetClassNodeMapping(Class);
		return true;
	};
}

void UBaseReplicationGraph::SetupAllReplicatedClasses()
{
	const auto RepGraphSettings = GetDefault<UReplicationGraphSettings>();
	check(RepGraphSettings);

	for (const FRepGraphActorClassSettings& ActorClassSettings : RepGraphSettings->ClassSettings)
	{
		if (!ActorClassSettings.bAddClassRepInfoToMap)
			continue;

		if (UClass* StaticActorClass = ActorClassSettings.GetStaticActorClass())
		{
			UE_LOG(LogRepGraph, Log, TEXT("ActorClassSettings -- AddClassRepInfo - %s :: %i"), *StaticActorClass->GetName(), int(ActorClassSettings.ClassNodeMapping));
			AddClassRepInfo(StaticActorClass, ActorClassSettings.ClassNodeMapping);
		}
	}

#if WITH_GAMEPLAY_DEBUGGER
	AddClassRepInfo(AGameplayDebuggerCategoryReplicator::StaticClass(), EClassRepNodeMapping::NotRouted);
#endif

	TArray<UClass*> AllReplicatedClasses;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		const AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
		if (!ActorCDO || !ActorCDO->GetIsReplicated())
			continue;

		if (Class->GetName().StartsWith(TEXT("SKEL_")) ||
			Class->GetName().StartsWith(TEXT("REINST_")))
			continue;

		AllReplicatedClasses.Add(Class);
		RegisterClassRepNodeMapping(Class);
	}

	for (UClass* ReplicatedClass : AllReplicatedClasses)
	{
		RegisterClassReplicationInfo(ReplicatedClass);
	}
}

void UBaseReplicationGraph::SetupCharacterClassReplicationInfo()
{
	FClassReplicationInfo CharacterClassRepInfo;
	CharacterClassRepInfo.DistancePriorityScale = 1.f;
	CharacterClassRepInfo.StarvationPriorityScale = 1.f;
	CharacterClassRepInfo.ActorChannelFrameTimeout = 4;
	CharacterClassRepInfo.SetCullDistanceSquared(ACharacter::StaticClass()->GetDefaultObject<ACharacter>()->GetNetCullDistanceSquared());

	ValidateSharedRepMovement();

	CharacterClassRepInfo.FastSharedReplicationFunc = [](AActor* Actor)
	{
		const auto CharacterSharedRep = Cast<ISharedReplication>(Actor);
		return CharacterSharedRep ? CharacterSharedRep->UpdateSharedReplication() : false;
	};

	CharacterClassRepInfo.FastSharedReplicationFuncName = FName(TEXT("FastSharedReplication"));

	FastSharedPathConstants.MaxBitsPerFrame = static_cast<int32>(static_cast<float>(ReplicationGraphConsole::TargetKBytesSecFastSharedPath * 1024 * 8) / NetDriver->GetNetServerMaxTickRate());
	FastSharedPathConstants.DistanceRequirementPct = ReplicationGraphConsole::FastSharedPathCullDistPct;

	GlobalActorReplicationInfoMap.SetClassInfo(ACharacter::StaticClass(), CharacterClassRepInfo);
}

void UBaseReplicationGraph::ValidateSharedRepMovement()
{
	FRepMovement DefaultRepMovement = ACharacter::StaticClass()->GetDefaultObject<ACharacter>()->GetReplicatedMovement();
	FSharedRepMovement SharedRepMovement;

	ensureMsgf(SharedRepMovement.RepMovement.LocationQuantizationLevel == DefaultRepMovement.LocationQuantizationLevel, TEXT("LocationQuantizationLevel mismatch. %d != %d"),
	           static_cast<uint8>(SharedRepMovement.RepMovement.LocationQuantizationLevel), static_cast<uint8>(DefaultRepMovement.LocationQuantizationLevel));
	ensureMsgf(SharedRepMovement.RepMovement.VelocityQuantizationLevel == DefaultRepMovement.VelocityQuantizationLevel, TEXT("VelocityQuantizationLevel mismatch. %d != %d"),
	           static_cast<uint8>(SharedRepMovement.RepMovement.VelocityQuantizationLevel), static_cast<uint8>(DefaultRepMovement.VelocityQuantizationLevel));
	ensureMsgf(SharedRepMovement.RepMovement.RotationQuantizationLevel == DefaultRepMovement.RotationQuantizationLevel, TEXT("RotationQuantizationLevel mismatch. %d != %d"),
	           static_cast<uint8>(SharedRepMovement.RepMovement.RotationQuantizationLevel), static_cast<uint8>(DefaultRepMovement.RotationQuantizationLevel));
}

void UBaseReplicationGraph::SetupRPCMulticastOpenChannelForClass()
{
	RPC_Multicast_OpenChannelForClass.Reset();
	RPC_Multicast_OpenChannelForClass.Set(AActor::StaticClass(), true);
	RPC_Multicast_OpenChannelForClass.Set(AController::StaticClass(), false);
	RPC_Multicast_OpenChannelForClass.Set(AServerStatReplicator::StaticClass(), false);

	const auto RepGraphSettings = GetDefault<UReplicationGraphSettings>();
	check(RepGraphSettings);

	for (const FRepGraphActorClassSettings& ActorClassSettings : RepGraphSettings->ClassSettings)
	{
		if (!ActorClassSettings.bAddToRPC_Multicast_OpenChannelForClassMap)
			continue;

		if (UClass* StaticActorClass = ActorClassSettings.GetStaticActorClass())
		{
			UE_LOG(LogRepGraph, Log, TEXT("ActorClassSettings -- RPC_Multicast_OpenChannelForClass - %s"), *StaticActorClass->GetName());
			RPC_Multicast_OpenChannelForClass.Set(StaticActorClass, ActorClassSettings.bRPC_Multicast_OpenChannelForClass);
		}
	}
}

void UBaseReplicationGraph::InitGlobalGraphNodes()
{
	// -----------------------------------------------
	//	Spatial Actors
	// -----------------------------------------------

	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = ReplicationGraphConsole::CellSize;
	GridNode->SpatialBias = FVector2D(ReplicationGraphConsole::SpatialBiasX, ReplicationGraphConsole::SpatialBiasY);

	if (ReplicationGraphConsole::DisableSpatialRebuilds)
		GridNode->AddToClassRebuildDenyList(AActor::StaticClass()); // Disable All spatial rebuilding

	AddGlobalGraphNode(GridNode);

	// -----------------------------------------------
	//	Always Relevant (to everyone) Actors
	// -----------------------------------------------
	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);

	// -----------------------------------------------
	//	Player State specialization. This will return a rolling subset of the player states to replicate
	// -----------------------------------------------
	const auto PlayerStateNode = CreateNewNode<UReplicationGraphNode_PlayerStateFrequencyLimiter>();
	AddGlobalGraphNode(PlayerStateNode);
}

void UBaseReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection)
{
	Super::InitConnectionGraphNodes(RepGraphConnection);

	UBaseReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantConnectionNode = CreateNewNode<UBaseReplicationGraphNode_AlwaysRelevant_ForConnection>();

	// This node needs to know when client levels go in and out of visibility
	RepGraphConnection->OnClientVisibleLevelNameAdd.AddUObject(AlwaysRelevantConnectionNode, &UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd);
	RepGraphConnection->OnClientVisibleLevelNameRemove.AddUObject(AlwaysRelevantConnectionNode, &UBaseReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove);

	AddConnectionGraphNode(AlwaysRelevantConnectionNode, RepGraphConnection);
}

EClassRepNodeMapping UBaseReplicationGraph::GetMappingPolicy(UClass* Class)
{
	const EClassRepNodeMapping* PolicyPtr = ClassRepNodePolicies.Get(Class);
	const EClassRepNodeMapping Policy = PolicyPtr ? *PolicyPtr : EClassRepNodeMapping::NotRouted;
	return Policy;
}

void UBaseReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	switch (GetMappingPolicy(ActorInfo.Class))
	{
	case EClassRepNodeMapping::NotRouted:
		break;

	case EClassRepNodeMapping::RelevantAllConnections:
		if (ActorInfo.StreamingLevelName != NAME_None)
		{
			FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindOrAdd(ActorInfo.StreamingLevelName);
			RepList.ConditionalAdd(ActorInfo.Actor);
		}
		else
			AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
		break;

	case EClassRepNodeMapping::Spatialize_Static:
		GridNode->AddActor_Static(ActorInfo, GlobalInfo);
		break;

	case EClassRepNodeMapping::Spatialize_Dynamic:
		GridNode->AddActor_Dynamic(ActorInfo, GlobalInfo);
		break;

	case EClassRepNodeMapping::Spatialize_Dormancy:
		GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
		break;
	}
}

void UBaseReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	switch (GetMappingPolicy(ActorInfo.Class))
	{
	case EClassRepNodeMapping::NotRouted:
		break;

	case EClassRepNodeMapping::RelevantAllConnections:
		if (ActorInfo.StreamingLevelName != NAME_None)
		{
			FActorRepListRefView& RepList = AlwaysRelevantStreamingLevelActors.FindChecked(ActorInfo.StreamingLevelName);
			if (RepList.RemoveFast(ActorInfo.Actor) == false)
			{
				UE_LOG(LogRepGraph, Warning, TEXT("Actor %s was not found in AlwaysRelevantStreamingLevelActors list. LevelName: %s"), *GetActorRepListTypeDebugString(ActorInfo.Actor),
				       *ActorInfo.StreamingLevelName.ToString());
			}
		}
		else
			AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);

		SetActorDestructionInfoToIgnoreDistanceCulling(ActorInfo.GetActor());
		break;

	case EClassRepNodeMapping::Spatialize_Static:
		GridNode->RemoveActor_Static(ActorInfo);
		break;

	case EClassRepNodeMapping::Spatialize_Dynamic:
		GridNode->RemoveActor_Dynamic(ActorInfo);
		break;

	case EClassRepNodeMapping::Spatialize_Dormancy:
		GridNode->RemoveActor_Dormancy(ActorInfo);
		break;
	}
}

// Since we listen to global (static) events, we need to watch out for cross world broadcasts (PIE)
#if WITH_EDITOR
#define CHECK_WORLDS(X) if(X->GetWorld() != GetWorld()) return;
#else
#define CHECK_WORLDS(X)
#endif

#if WITH_GAMEPLAY_DEBUGGER
void UBaseReplicationGraph::OnGameplayDebuggerOwnerChange(AGameplayDebuggerCategoryReplicator* Debugger, APlayerController* OldOwner)
{
	CHECK_WORLDS(Debugger);

	auto GetAlwaysRelevantForConnectionNode = [this](const APlayerController* Controller) -> UBaseReplicationGraphNode_AlwaysRelevant_ForConnection*
	{
		if (!Controller)
			return nullptr;

		const auto NetConnection = Controller->GetNetConnection();
		if (!NetConnection || NetConnection->GetDriver() != NetDriver)
			return nullptr;

		const auto GraphConnection = FindOrAddConnectionManager(NetConnection);
		if (!GraphConnection)
			return nullptr;

		for (UReplicationGraphNode* ConnectionNode : GraphConnection->GetConnectionGraphNodes())
		{
			if (const auto AlwaysRelevantConnectionNode = Cast<UBaseReplicationGraphNode_AlwaysRelevant_ForConnection>(ConnectionNode))
				return AlwaysRelevantConnectionNode;
		}
		// We should always have a connection node for the AlwaysRelevantConnectionNode
		// If we get here, we didn't find the AlwaysRelevantConnectionNode
		return nullptr;
	};

	if (const auto AlwaysRelevantConnectionNode = GetAlwaysRelevantForConnectionNode(OldOwner))
		AlwaysRelevantConnectionNode->GameplayDebugger = nullptr;

	if (const auto AlwaysRelevantConnectionNode = GetAlwaysRelevantForConnectionNode(Debugger->GetReplicationOwner()))
		AlwaysRelevantConnectionNode->GameplayDebugger = Debugger;
}
#endif

#undef CHECK_WORLDS


void UBaseReplicationGraph::PrintRepNodePolicies()
{
	const UEnum* Enum = StaticEnum<EClassRepNodeMapping>();
	if (!Enum)
		return;

	GLog->Logf(TEXT("===================================="));
	GLog->Logf(TEXT(" Replication Routing Policies"));
	GLog->Logf(TEXT("===================================="));

	for (auto It = ClassRepNodePolicies.CreateIterator(); It; ++It)
	{
		FObjectKey ObjKey = It.Key();
		EClassRepNodeMapping Mapping = It.Value();
		GLog->Logf(TEXT("%-40s --> %s"), *GetNameSafe(ObjKey.ResolveObjectPtr()), *Enum->GetNameStringByValue(static_cast<uint32>(Mapping)));
	}
}
