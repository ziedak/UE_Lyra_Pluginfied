// Copyright Epic Games, Inc. All Rights Reserved.

#include "AsyncAction_CommonUserInitialize.h"

#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_CommonUserInitialize)

UAsyncAction_CommonUserInitialize* UAsyncAction_CommonUserInitialize::InitializeForLocalPlay(
	UCommonUserSubsystem* Target,
	const int32 LocalPlayerIndex,
	FInputDeviceId PrimaryInputDevice,
	const bool bCanUseGuestLogin)
{
	if (!PrimaryInputDevice.IsValid())
	{
		// Set to default device
		PrimaryInputDevice = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
	}

	UAsyncAction_CommonUserInitialize* Action = NewObject<UAsyncAction_CommonUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;

		Action->Params.RequestedPrivilege = ECommonUserPrivilege::CanPlay;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.PrimaryInputDevice = PrimaryInputDevice;
		Action->Params.bCanUseGuestLogin = bCanUseGuestLogin;
		Action->Params.bCanCreateNewLocalPlayer = true;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

UAsyncAction_CommonUserInitialize* UAsyncAction_CommonUserInitialize::LoginForOnlinePlay(
	UCommonUserSubsystem* Target,
	int32 LocalPlayerIndex)
{
	UAsyncAction_CommonUserInitialize* Action = NewObject<UAsyncAction_CommonUserInitialize>();

	Action->RegisterWithGameInstance(Target);

	if (Target && Action->IsRegistered())
	{
		Action->Subsystem = Target;

		Action->Params.RequestedPrivilege = ECommonUserPrivilege::CanPlayOnline;
		Action->Params.LocalPlayerIndex = LocalPlayerIndex;
		Action->Params.bCanCreateNewLocalPlayer = false;
	}
	else
	{
		Action->SetReadyToDestroy();
	}

	return Action;
}

void UAsyncAction_CommonUserInitialize::HandleFailure()
{
	const UCommonUserInfo* UserInfo = nullptr;
	if (Subsystem.IsValid())
	{
		UserInfo = Subsystem->GetUserInfoForLocalPlayerIndex(Params.LocalPlayerIndex);
	}
	HandleInitializationComplete(UserInfo, false,
	                             NSLOCTEXT("CommonUser", "LoginFailedEarly", "Unable to start login process"),
	                             Params.RequestedPrivilege, Params.OnlineContext);
}

void UAsyncAction_CommonUserInitialize::HandleInitializationComplete(const UCommonUserInfo* UserInfo,
                                                                     const bool bSuccess,
                                                                     const FText Error,
                                                                     const ECommonUserPrivilege RequestedPrivilege,
                                                                     const ECommonUserOnlineContext OnlineContext)
{
	if (ShouldBroadcastDelegates())
	{
		OnInitializationComplete.Broadcast(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);
	}

	SetReadyToDestroy();
}

void UAsyncAction_CommonUserInitialize::Activate()
{
	if (!Subsystem.IsValid())
	{
		SetReadyToDestroy();
		return;
	}

	Params.OnUserInitializeComplete.BindUFunction(
		this, GET_FUNCTION_NAME_CHECKED(UAsyncAction_CommonUserInitialize, HandleInitializationComplete));

	if (!Subsystem->TryToInitializeUser(Params))
	{
		// Call failure next frame
		if (FTimerManager* TimerManager = GetTimerManager())
		{
			TimerManager->SetTimerForNextTick(
				FTimerDelegate::CreateUObject(this, &UAsyncAction_CommonUserInitialize::HandleFailure));
		}
	}
}
