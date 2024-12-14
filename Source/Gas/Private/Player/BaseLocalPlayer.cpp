// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BaseLocalPlayer.h"

#include "AudioMixerBlueprintLibrary.h"
#include "Settings/LyraSettingsShared.h"
#include "Settings/LyraSettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseLocalPlayer)

void UBaseLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();

	if (ULyraSettingsLocal* LocalSettings = GetLocalSettings())
		LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &ThisClass::OnAudioOutputDeviceChanged);
}

void UBaseLocalPlayer::SwitchController(APlayerController* PC)
{
	Super::SwitchController(PC);
	OnPlayerControllerChanged(PC);
}

bool UBaseLocalPlayer::SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld)
{
	const bool bResult = Super::SpawnPlayActor(URL, OutError, InWorld);
	OnPlayerControllerChanged(PlayerController);

	return bResult;
}

void UBaseLocalPlayer::InitOnlineSession()
{
	OnPlayerControllerChanged(PlayerController);
	Super::InitOnlineSession();
}

ULyraSettingsLocal* UBaseLocalPlayer::GetLocalSettings() const
{
	return ULyraSettingsLocal::Get();
}

void UBaseLocalPlayer::OnPlayerControllerChanged(APlayerController* NewController)
{
	// 	// Stop listening for changes from the old controller
	// 	FGenericTeamId OldTeamID = FGenericTeamId::NoTeam;
	// 	if (IBaseTeamAgentInterface* ControllerAsTeamProvider = Cast<IBaseTeamAgentInterface>(LastBoundPC.Get()))
	// 	{
	// 		OldTeamID = ControllerAsTeamProvider->GetGenericTeamId();
	// 		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().RemoveAll(this);
	// 	}
	//
	// 	// Grab the current team ID and listen for future changes
	// 	FGenericTeamId NewTeamID = FGenericTeamId::NoTeam;
	// 	if (IBaseTeamAgentInterface* ControllerAsTeamProvider = Cast<IBaseTeamAgentInterface>(NewController))
	// 	{
	// 		NewTeamID = ControllerAsTeamProvider->GetGenericTeamId();
	// 		ControllerAsTeamProvider->GetTeamChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	// 		LastBoundPC = NewController;
	// 	}
	//
	// 	ConditionalBroadcastTeamChanged(this, OldTeamID, NewTeamID);
}

ULyraSettingsShared* UBaseLocalPlayer::GetSharedSettings() const
{
	if (SharedSettings)
		return SharedSettings;

	// On PC it's okay to use the sync load because it only checks the disk
	// This could use a platform tag to check for proper save support instead
	const bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;
	if (bCanLoadBeforeLogin)
		return ULyraSettingsShared::LoadOrCreateSettings(this);

	// We need to wait for user login to get the real settings so return temp ones
	return ULyraSettingsShared::CreateTemporarySettings(this);
}

void UBaseLocalPlayer::LoadSharedSettingsFromDisk(const bool bForceLoad)
{
	// Already loaded once, don't reload
	if (!bForceLoad && SharedSettings && GetCachedUniqueNetId() == NetIdForSharedSettings)
		return;

	ensure(
		ULyraSettingsShared::AsyncLoadOrCreateSettings(this,
			ULyraSettingsShared::FOnSettingsLoadedEvent::CreateUObject(
				this, &UBaseLocalPlayer::OnSharedSettingsLoaded)));
}

void UBaseLocalPlayer::OnSharedSettingsLoaded(ULyraSettingsShared* LoadedOrCreatedSettings)
{
	// The settings are applied before it gets here
	if (!(ensure(LoadedOrCreatedSettings)))
		return;

	// This will replace the temporary or previously loaded object which will GC out normally
	SharedSettings = LoadedOrCreatedSettings;
	NetIdForSharedSettings = GetCachedUniqueNetId();
}

void UBaseLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(),
	                                                   InAudioOutputDeviceId,
	                                                   DevicesSwappedCallback);
}

void UBaseLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
		UE_LOG(LogTemp, Error, TEXT("Failed to swap audio device: %s"), *SwapResult.CurrentDeviceId);
}
