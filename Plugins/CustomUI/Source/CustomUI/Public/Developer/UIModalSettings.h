// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "UIModalSettings.generated.h"

class UCommonGameDialog;
/**
 *
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName="UI Modal Screen"))
class CUSTOMUI_API UUIModalSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UUIModalSettings();

	UPROPERTY(config, EditAnywhere, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath ConfirmationDialogClass;

	UPROPERTY(config, EditAnywhere, meta=(MetaClass="/Script/UMG.UserWidget"))
	FSoftClassPath ErrorDialogClass;
};
