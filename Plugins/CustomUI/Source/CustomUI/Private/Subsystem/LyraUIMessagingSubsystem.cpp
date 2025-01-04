// Copyright Epic Games, Inc. All Rights Reserved.

#include "Subsystem/LyraUIMessagingSubsystem.h"

#include "Messaging/CommonGameDialog.h"
#include "NativeGameplayTags.h"
#include "CommonLocalPlayer.h"
#include "PrimaryGameLayout.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraUIMessagingSubsystem)

class FSubsystemCollectionBase;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MODAL, "UI.Layer.Modal");

void ULyraUIMessagingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ConfirmationDialogClassPtr = ConfirmationDialogClass.LoadSynchronous();
	ErrorDialogClassPtr = ErrorDialogClass.LoadSynchronous();
}

void ULyraUIMessagingSubsystem::ShowConfirmation(UCommonGameDialogDescriptor* DialogDescriptor,
                                                 FCommonMessagingResultDelegate ResultCallback)
{
	ShowDialog(TAG_UI_LAYER_MODAL, ConfirmationDialogClassPtr,
	           DialogDescriptor, ResultCallback);
}

void ULyraUIMessagingSubsystem::ShowError(UCommonGameDialogDescriptor* DialogDescriptor,
                                          FCommonMessagingResultDelegate ResultCallback)
{
	ShowDialog(TAG_UI_LAYER_MODAL, ErrorDialogClassPtr,
	           DialogDescriptor, ResultCallback);
}

void ULyraUIMessagingSubsystem::ShowDialog(const FGameplayTag DialogTag,
                                           const TSubclassOf<UCommonGameDialog>& DialogClassPtr,
                                           UCommonGameDialogDescriptor* DialogDescriptor,
                                           FCommonMessagingResultDelegate ResultCallback)
{
	const auto LocalPlayer = GetLocalPlayer<UCommonLocalPlayer>();
	if (!LocalPlayer) return;

	const auto RootLayout = LocalPlayer->GetRootUILayout();
	if (!RootLayout) return;

	RootLayout->PushWidgetToLayerStack<UCommonGameDialog>(DialogTag,
	                                                      DialogClassPtr,
	                                                      [DialogDescriptor, ResultCallback](
	                                                      UCommonGameDialog& Dialog){
		                                                      Dialog.SetupDialog(DialogDescriptor, ResultCallback);
	                                                      });
}