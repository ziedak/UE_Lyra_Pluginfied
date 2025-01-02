// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GGameInstance.h"

#include "CommonUserSubsystem.h"
#include "Player/BaseLocalPlayer.h"


void UGGameInstance::HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess,
                                            FText Error, ECommonUserPrivilege RequestedPrivilege,
                                            ECommonUserOnlineContext OnlineContext)
{
	Super::HandlerUserInitialized(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);

	// If login succeeded, tell the local player to load their settings
	if (!bSuccess || !ensure(UserInfo)) { return; }

	// There will not be a local player attached to the dedicated server user
	if (const auto LocalPlayer = Cast<UBaseLocalPlayer>(GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex)))
	{
		LocalPlayer->LoadSharedSettingsFromDisk();
	}
}
