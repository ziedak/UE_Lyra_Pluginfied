// Copyright Epic Games, Inc. All Rights Reserved.

#include "Performance/LyraPerformanceStatSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
// #include "GameModes/LyraGameState.h"
#include "Interfaces/IGameStateFpsInterface.h"
#include "Performance/LyraPerformanceStatTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPerformanceStatSubsystem)

class FSubsystemCollectionBase;

//////////////////////////////////////////////////////////////////////
// FLyraPerformanceStatCache

void FLyraPerformanceStatCache::StartCharting() {}

void FLyraPerformanceStatCache::ProcessFrame(const FFrameData& FrameData)
{
	CachedData = FrameData;
	CachedServerFPS = 0.0f;
	CachedPingMS = 0.0f;
	CachedPacketLossIncomingPercent = 0.0f;
	CachedPacketLossOutgoingPercent = 0.0f;
	CachedPacketRateIncoming = 0.0f;
	CachedPacketRateOutgoing = 0.0f;
	CachedPacketSizeIncoming = 0.0f;
	CachedPacketSizeOutgoing = 0.0f;

	UWorld* World = MySubsystem->GetGameInstance()->GetWorld();
	if (!World) return;

	const auto GameState = World->GetGameState();
	if (GameState && GameState->Implements<UGameStateFpsInterface>()) if (const auto IGameState = CastChecked<IGameStateFpsInterface>(GameState)) CachedServerFPS = IGameState->GetServerFPS();
	//
	// if (const ALyraGameState* GameState = World->GetGameState<ALyraGameState>())
	// {
	// 	CachedServerFPS = GameState->GetServerFPS();
	// }


	const APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(World);
	if (!LocalPC) return;

	if (const APlayerState* PS = LocalPC->GetPlayerState<APlayerState>()) CachedPingMS = PS->GetPingInMilliseconds();

	const UNetConnection* NetConnection = LocalPC->GetNetConnection();
	if (!NetConnection) return;

	const UNetConnection::FNetConnectionPacketLoss& InLoss = NetConnection->GetInLossPercentage();
	CachedPacketLossIncomingPercent = InLoss.GetAvgLossPercentage();
	const UNetConnection::FNetConnectionPacketLoss& OutLoss = NetConnection->GetOutLossPercentage();
	CachedPacketLossOutgoingPercent = OutLoss.GetAvgLossPercentage();

	CachedPacketRateIncoming = NetConnection->InPacketsPerSecond;
	CachedPacketRateOutgoing = NetConnection->OutPacketsPerSecond;

	CachedPacketSizeIncoming = (NetConnection->InPacketsPerSecond != 0)
		                           ? (NetConnection->InBytesPerSecond / static_cast<float>(NetConnection->
			                           InPacketsPerSecond))
		                           : 0.0f;
	CachedPacketSizeOutgoing = (NetConnection->OutPacketsPerSecond != 0)
		                           ? (NetConnection->OutBytesPerSecond / static_cast<float>(NetConnection->
			                           OutPacketsPerSecond))
		                           : 0.0f;
}

void FLyraPerformanceStatCache::StopCharting() {}

double FLyraPerformanceStatCache::GetCachedStat(EDisplayablePerformanceStat Stat) const
{
	static_assert(static_cast<int32>(EDisplayablePerformanceStat::Count) == 15,
	              "Need to update this function to deal with new performance stats");
	switch (Stat)
	{
	case EDisplayablePerformanceStat::ClientFPS: return (CachedData.TrueDeltaSeconds != 0.0) ? (1.0 / CachedData.TrueDeltaSeconds) : 0.0;
	case EDisplayablePerformanceStat::ServerFPS: return CachedServerFPS;
	case EDisplayablePerformanceStat::IdleTime: return CachedData.IdleSeconds;
	case EDisplayablePerformanceStat::FrameTime: return CachedData.TrueDeltaSeconds;
	case EDisplayablePerformanceStat::FrameTime_GameThread: return CachedData.GameThreadTimeSeconds;
	case EDisplayablePerformanceStat::FrameTime_RenderThread: return CachedData.RenderThreadTimeSeconds;
	case EDisplayablePerformanceStat::FrameTime_RHIThread: return CachedData.RHIThreadTimeSeconds;
	case EDisplayablePerformanceStat::FrameTime_GPU: return CachedData.GPUTimeSeconds;
	case EDisplayablePerformanceStat::Ping: return CachedPingMS;
	case EDisplayablePerformanceStat::PacketLoss_Incoming: return CachedPacketLossIncomingPercent;
	case EDisplayablePerformanceStat::PacketLoss_Outgoing: return CachedPacketLossOutgoingPercent;
	case EDisplayablePerformanceStat::PacketRate_Incoming: return CachedPacketRateIncoming;
	case EDisplayablePerformanceStat::PacketRate_Outgoing: return CachedPacketRateOutgoing;
	case EDisplayablePerformanceStat::PacketSize_Incoming: return CachedPacketSizeIncoming;
	case EDisplayablePerformanceStat::PacketSize_Outgoing: return CachedPacketSizeOutgoing;
	case EDisplayablePerformanceStat::Count: break;
	default: return 0.0f;
	}


	return 0.0f;
}

//////////////////////////////////////////////////////////////////////
// ULyraPerformanceStatSubsystem

void ULyraPerformanceStatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Tracker = MakeShared<FLyraPerformanceStatCache>(this);
	GEngine->AddPerformanceDataConsumer(Tracker);
}

void ULyraPerformanceStatSubsystem::Deinitialize()
{
	GEngine->RemovePerformanceDataConsumer(Tracker);
	Tracker.Reset();
}

double ULyraPerformanceStatSubsystem::GetCachedStat(EDisplayablePerformanceStat Stat) const { return Tracker->GetCachedStat(Stat); }
