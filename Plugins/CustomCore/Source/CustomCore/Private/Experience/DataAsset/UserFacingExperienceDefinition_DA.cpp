// Fill out your copyright notice in the Description page of Project Settings.


#include "Experience/DataAsset/UserFacingExperienceDefinition_DA.h"
#include "CommonSessionSubsystem.h"

UCommonSession_HostSessionRequest* UUserFacingExperienceDefinition_DA::CreateHostingRequest(
	const UObject* WorldContextObject) const
{
	const FString ExperienceName = ExperienceID.PrimaryAssetName.ToString();
	const FString UserFacingExperienceName = GetPrimaryAssetId().PrimaryAssetName.ToString();

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UCommonSession_HostSessionRequest* HostSessionRequest = nullptr;
	UCommonSessionSubsystem* SessionSubsystem = GameInstance
		                                            ? GameInstance->GetSubsystem<UCommonSessionSubsystem>()
		                                            : nullptr;
	if (SessionSubsystem) HostSessionRequest = SessionSubsystem->CreateOnlineHostSessionRequest();

	if (!HostSessionRequest)
	{
		// Couldn't use the subsystem so create one
		HostSessionRequest = NewObject<UCommonSession_HostSessionRequest>();
		HostSessionRequest->OnlineMode = ECommonSessionOnlineMode::Online;
		HostSessionRequest->bUseLobbies = true;
		//HostSessionRequest->bUseLobbiesVoiceChat = false;
		// We always enable presence on this session because it is the primary session used for matchmaking. For online systems that care about presence, only the primary session should have presence enabled
		//HostSessionRequest->bUsePresence = !IsRunningDedicatedServer();
	}
	HostSessionRequest->MapID = MapID;
	HostSessionRequest->ModeNameForAdvertisement = UserFacingExperienceName;
	HostSessionRequest->ExtraArgs = ExtraArgs;
	HostSessionRequest->ExtraArgs.Add(TEXT("Experience"), ExperienceName);
	HostSessionRequest->MaxPlayerCount = MaxPlayerCount;

	// if (ULyraReplaySubsystem::DoesPlatformSupportReplays())
	// {
	// 	if (bRecordReplay)
	// 	{
	// 		HostSessionRequest->ExtraArgs.Add(TEXT("DemoRec"), FString());
	// 	}
	// }

	return HostSessionRequest;
}