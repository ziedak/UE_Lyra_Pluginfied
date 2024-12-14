// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "PlayerSharedSettings.generated.h"

class ULyraSettingsLocal;
class ULyraSettingsShared;

UINTERFACE()
class UPlayerSharedSettings : public UInterface
{
	GENERATED_BODY()
};

class GAMELOCALSETTINGS_API IPlayerSharedSettings
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual ULyraSettingsShared* GetSharedSettings() const =0;
	UFUNCTION()
	virtual ULyraSettingsLocal* GetLocalSettings() const =0;
};
