// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameSettingFilterState.h"

class ULocalPlayer;

class GAMESETTINGS_API FWhenPlatformSupportsMouseAndKeyboard : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenPlatformSupportsMouseAndKeyboard> Get();

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override;
};
