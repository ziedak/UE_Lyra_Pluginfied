// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "IPlayerSharedSettingsInterface.generated.h"

class ULyraSettingsLocal;
class ULyraSettingsShared;

UINTERFACE()
class UPlayerSharedSettingsInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMELOCALSETTINGS_API IPlayerSharedSettingsInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual ULyraSettingsShared* GetSharedSettings() const =0;
	UFUNCTION()
	virtual ULyraSettingsLocal* GetLocalSettings() const =0;
};
