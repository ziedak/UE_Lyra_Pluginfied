// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameSettingFilterState.h"

class ULocalPlayer;
/**
 * Checks Auto quality not supported
 */

class FWhenAutoQualitySupported : public FGameSettingEditCondition
{
public:
	static TSharedRef<FWhenAutoQualitySupported> KillIfNot();

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer,
	                             FGameSettingEditableState& InOutEditState) const override;
};
