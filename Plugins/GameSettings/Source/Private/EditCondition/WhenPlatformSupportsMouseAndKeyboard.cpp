// Fill out your copyright notice in the Description page of Project Settings.


#include "EditCondition/WhenPlatformSupportsMouseAndKeyboard.h"

#include "CommonInputBaseTypes.h"

#define LOCTEXT_NAMESPACE "GameSetting"

TSharedRef<FWhenPlatformSupportsMouseAndKeyboard> FWhenPlatformSupportsMouseAndKeyboard::Get()
{
	static TSharedRef<FWhenPlatformSupportsMouseAndKeyboard> Instance = MakeShared<
		FWhenPlatformSupportsMouseAndKeyboard>();
	return Instance;
}

void FWhenPlatformSupportsMouseAndKeyboard::GatherEditState(const ULocalPlayer* InLocalPlayer,
                                                            FGameSettingEditableState& InOutEditState) const
{
	const UCommonInputPlatformSettings* PlatformInput = UPlatformSettingsManager::Get().GetSettingsForPlatform<
		UCommonInputPlatformSettings>();
	if (!PlatformInput->SupportsInputType(ECommonInputType::MouseAndKeyboard))
	{
		InOutEditState.Kill(TEXT("Platform does not support mouse and keyboard"));
	}
}

#undef LOCTEXT_NAMESPACE
