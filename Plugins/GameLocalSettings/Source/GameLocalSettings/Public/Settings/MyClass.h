// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "MyClass.generated.h"

class ULyraSettingsLocal;
class ULyraSettingsShared;
/**
 *
 */
UCLASS()
class GAMELOCALSETTINGS_API UMyClass : public ULocalPlayer
{
	GENERATED_BODY()

public :
	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	ULyraSettingsLocal* GetLocalSettings() const;

	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	UFUNCTION()
	ULyraSettingsShared* GetSharedSettings() const;

	UPROPERTY(Transient)
	mutable TObjectPtr<ULyraSettingsShared> SharedSettings;
	UPROPERTY()
	bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;
};
