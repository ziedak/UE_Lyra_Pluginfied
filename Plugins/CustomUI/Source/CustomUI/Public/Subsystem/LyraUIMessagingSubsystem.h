// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"

#include "Messaging/CommonMessagingSubsystem.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"

#include "LyraUIMessagingSubsystem.generated.h"

class FSubsystemCollectionBase;
class UCommonGameDialog;
class UCommonGameDialogDescriptor;
class UObject;

/**
 * 
 */
UCLASS()
class ULyraUIMessagingSubsystem : public UCommonMessagingSubsystem
{
	GENERATED_BODY()

public:
	ULyraUIMessagingSubsystem() {}

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate()) override;
	virtual void ShowError(UCommonGameDialogDescriptor* DialogDescriptor, FCommonMessagingResultDelegate ResultCallback = FCommonMessagingResultDelegate()) override;

private:
	virtual void ShowDialog(const FGameplayTag DialogTag,
	                        const TSubclassOf<UCommonGameDialog>& DialogClassPtr,
	                        UCommonGameDialogDescriptor* DialogDescriptor,
	                        FCommonMessagingResultDelegate ResultCallback);
	UPROPERTY()
	TSubclassOf<UCommonGameDialog> ConfirmationDialogClassPtr;

	UPROPERTY()
	TSubclassOf<UCommonGameDialog> ErrorDialogClassPtr;

	UPROPERTY(config)
	TSoftClassPtr<UCommonGameDialog> ConfirmationDialogClass;

	UPROPERTY(config)
	TSoftClassPtr<UCommonGameDialog> ErrorDialogClass;
};