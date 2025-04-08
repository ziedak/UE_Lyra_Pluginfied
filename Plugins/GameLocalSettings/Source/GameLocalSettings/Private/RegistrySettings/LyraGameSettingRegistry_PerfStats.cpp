// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSettings/LyraSettingValueDiscrete_PerfStat.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "GameSettingCollection.h"
#include "RegistrySettings/LyraGameSettingRegistry.h"
#include "Performance/LyraPerformanceStatTypes.h"

class ULyraLocalPlayer;

#define LOCTEXT_NAMESPACE "Lyra"


void ULyraGameSettingRegistry::AddPerformanceStatPage(UGameSettingCollection* PerfStatsOuterCategory) const
{
	static_assert(static_cast<int32>(EDisplayablePerformanceStat::Count) == 15,
	              "Consider updating this function to deal with new performance stats");

	const auto StatsPage = CreatePerformanceStatsPage();
	PerfStatsOuterCategory->AddSetting(StatsPage);

	AddPerformanceStats(StatsPage);
	AddNetworkStats(StatsPage);
}

UGameSettingCollectionPage* ULyraGameSettingRegistry::CreatePerformanceStatsPage() const
{
	return UGameSettingCollectionPage::CreateSettings(
		"PerfStatsPage",
		LOCTEXT("PerfStatsPage_Name", "Performance Stats"),
		LOCTEXT("PerfStatsPage_Description", "Configure the display of performance statistics."),
		LOCTEXT("PerfStatsPage_Navigation", "Edit"),
		FWhenPlayingAsPrimaryPlayer::Get());
}

void ULyraGameSettingRegistry::AddPerformanceStats(UGameSettingCollectionPage* StatsPage) const
{
	const auto StatCategory_Performance = UGameSettingCollection::CreateCollection(
		"StatCategory_Performance", LOCTEXT("StatCategory_Performance_Name", "Performance"));
	StatsPage->AddSetting(StatCategory_Performance);

	const auto ClientFPS = AddPerformanceStat(EDisplayablePerformanceStat::ClientFPS,
	                                          LOCTEXT("PerfStat_ClientFPS", "Client FPS"),
	                                          LOCTEXT("PerfStatDescription_ClientFPS", "Client frame rate (higher is better)"));
	StatCategory_Performance->AddSetting(ClientFPS);

	const auto ServerFPS = AddPerformanceStat(EDisplayablePerformanceStat::ServerFPS,
	                                          LOCTEXT("PerfStat_ServerFPS", "Server FPS"),
	                                          LOCTEXT("PerfStatDescription_ServerFPS", "Server frame rate"));
	StatCategory_Performance->AddSetting(ServerFPS);

	const auto FrameTime = AddPerformanceStat(EDisplayablePerformanceStat::FrameTime,
	                                          LOCTEXT("PerfStat_FrameTime", "Frame Time"),
	                                          LOCTEXT("PerfStatDescription_FrameTime", "The total frame time."));
	StatCategory_Performance->AddSetting(FrameTime);

	const auto IdleTime = AddPerformanceStat(EDisplayablePerformanceStat::IdleTime,
	                                         LOCTEXT("PerfStat_IdleTime", "Idle Time"),
	                                         LOCTEXT("PerfStatDescription_IdleTime", "The amount of time spent waiting idle for frame pacing."));
	StatCategory_Performance->AddSetting(IdleTime);
	const auto GameThread = AddPerformanceStat(EDisplayablePerformanceStat::FrameTime_GameThread,
	                                           LOCTEXT("PerfStat_FrameTime_GameThread", "CPU Game Time"),
	                                           LOCTEXT("PerfStatDescription_FrameTime_GameThread", "The amount of time spent on the main game thread."));
	StatCategory_Performance->AddSetting(GameThread);

	const auto FrameTime_RenderThread = AddPerformanceStat(EDisplayablePerformanceStat::FrameTime_RenderThread,
	                                                       LOCTEXT("PerfStat_FrameTime_RenderThread", "CPU Render Time"),
	                                                       LOCTEXT("PerfStatDescription_FrameTime_RenderThread", "The amount of time spent on the rendering thread."));
	StatCategory_Performance->AddSetting(FrameTime_RenderThread);

	const auto FrameTime_RHIThread = AddPerformanceStat(EDisplayablePerformanceStat::FrameTime_RHIThread,
	                                                    LOCTEXT("PerfStat_FrameTime_RHIThread", "CPU RHI Time"),
	                                                    LOCTEXT("PerfStatDescription_FrameTime_RHIThread", "The amount of time spent on the Render Hardware Interface thread."));
	StatCategory_Performance->AddSetting(FrameTime_RHIThread);

	const auto FrameTime_GPU = AddPerformanceStat(EDisplayablePerformanceStat::FrameTime_GPU,
	                                              LOCTEXT("PerfStat_FrameTime_GPU", "GPU Render Time"),
	                                              LOCTEXT("PerfStatDescription_FrameTime_GPU", "The amount of time spent on the GPU."));
	StatCategory_Performance->AddSetting(FrameTime_GPU);
}

void ULyraGameSettingRegistry::AddNetworkStats(UGameSettingCollectionPage* StatsPage) const
{
	UGameSettingCollection* StatCategory_Network = NewObject<UGameSettingCollection>();
	StatCategory_Network->SetDevName(TEXT("StatCategory_Network"));
	StatCategory_Network->SetDisplayName(LOCTEXT("StatCategory_Network_Name", "Network"));
	StatsPage->AddSetting(StatCategory_Network);

	const auto Ping = AddPerformanceStat(EDisplayablePerformanceStat::Ping,
	                                     LOCTEXT("PerfStat_Ping", "Ping"),
	                                     LOCTEXT("PerfStatDescription_Ping", "The roundtrip latency of your connection to the server."));
	StatCategory_Network->AddSetting(Ping);

	const auto PacketLoss_Incoming = AddPerformanceStat(EDisplayablePerformanceStat::PacketLoss_Incoming,
	                                                    LOCTEXT("PerfStat_PacketLoss_Incoming", "Incoming Packet Loss"),
	                                                    LOCTEXT("PerfStatDescription_PacketLoss_Incoming", "The percentage of incoming packets lost."));
	StatCategory_Network->AddSetting(PacketLoss_Incoming);

	const auto PacketLoss_Outgoing = AddPerformanceStat(EDisplayablePerformanceStat::PacketLoss_Outgoing,
	                                                    LOCTEXT("PerfStat_PacketLoss_Outgoing", "Outgoing Packet Loss"),
	                                                    LOCTEXT("PerfStatDescription_PacketLoss_Outgoing", "The percentage of outgoing packets lost."));
	StatCategory_Network->AddSetting(PacketLoss_Outgoing);

	const auto PacketRate_Incoming = AddPerformanceStat(EDisplayablePerformanceStat::PacketRate_Incoming,
	                                                    LOCTEXT("PerfStat_PacketRate_Incoming", "Incoming Packet Rate"),
	                                                    LOCTEXT("PerfStatDescription_PacketRate_Incoming", "Rate of incoming packets (per second)"));
	StatCategory_Network->AddSetting(PacketRate_Incoming);

	const auto PacketRate_Outgoing = AddPerformanceStat(EDisplayablePerformanceStat::PacketRate_Outgoing,
	                                                    LOCTEXT("PerfStat_PacketRate_Outgoing", "Outgoing Packet Rate"),
	                                                    LOCTEXT("PerfStatDescription_PacketRate_Outgoing", "Rate of outgoing packets (per second)"));
	StatCategory_Network->AddSetting(PacketRate_Outgoing);

	const auto PacketSize_Incoming = AddPerformanceStat(EDisplayablePerformanceStat::PacketSize_Incoming,
	                                                    LOCTEXT("PerfStat_PacketSize_Incoming", "Incoming Packet Size"),
	                                                    LOCTEXT("PerfStatDescription_PacketSize_Incoming", "The average size (in bytes) of packets received in the last second."));
	StatCategory_Network->AddSetting(PacketSize_Incoming);

	const auto PacketSize_Outgoing = AddPerformanceStat(EDisplayablePerformanceStat::PacketSize_Outgoing,
	                                                    LOCTEXT("PerfStat_PacketSize_Outgoing", "Outgoing Packet Size"),
	                                                    LOCTEXT("PerfStatDescription_PacketSize_Outgoing", "The average size (in bytes) of packets sent in the last second."));
	StatCategory_Network->AddSetting(PacketSize_Outgoing);
}

ULyraSettingValueDiscrete_PerfStat* ULyraGameSettingRegistry::AddPerformanceStat(const EDisplayablePerformanceStat Stat,
                                                                                 const FText& DisplayName,
                                                                                 const FText& Description) const
{
	const auto Setting = NewObject<ULyraSettingValueDiscrete_PerfStat>();
	Setting->SetStat(Stat);
	Setting->SetDisplayName(DisplayName);
	Setting->SetDescriptionRichText(Description);
	return Setting;
}

#undef LOCTEXT_NAMESPACE