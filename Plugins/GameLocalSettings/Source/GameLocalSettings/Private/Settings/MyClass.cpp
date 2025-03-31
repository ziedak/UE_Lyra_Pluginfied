// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/MyClass.h"

#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"


ULyraSettingsLocal* UMyClass::GetLocalSettings() const { return ULyraSettingsLocal::Get(); }

ULyraSettingsShared* UMyClass::GetSharedSettings() const
{
	if (SharedSettings) return SharedSettings;

	// On PC, it's okay to use the sync load because it only checks the disk,
	// This could use a platform tag to check for proper save support instead

	if (bCanLoadBeforeLogin) return ULyraSettingsShared::LoadOrCreateSettings(this);

	// We need to wait for user login to get the real settings so return temp ones
	return ULyraSettingsShared::CreateTemporarySettings(this);
}
