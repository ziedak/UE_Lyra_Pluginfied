// Copyright Epic Games, Inc. All Rights Reserved.

#include "Ui/Screens/LyraSettingScreen.h"
#include "Input/CommonUIInputTypes.h"
// #include "RegistrySettings/LyraGameSettingRegistry.h"
#include "GameSettingRegistry.h"

#include "RegistrySettings/LyraGameSettingRegistry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSettingScreen)

// class UGameSettingRegistry;

void ULyraSettingScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BackHandle = RegisterUIActionBinding(FBindUIActionArgs(BackInputActionData,
	                                                       true, FSimpleDelegate::CreateUObject(
		                                                       this, &ThisClass::HandleBackAction)));
	ApplyHandle = RegisterUIActionBinding(FBindUIActionArgs(ApplyInputActionData, true,
	                                                        FSimpleDelegate::CreateUObject(
		                                                        this, &ThisClass::HandleApplyAction)));
	CancelChangesHandle = RegisterUIActionBinding(FBindUIActionArgs(CancelChangesInputActionData, true,
	                                                                FSimpleDelegate::CreateUObject(
		                                                                this, &ThisClass::HandleCancelChangesAction)));
}

UGameSettingRegistry* ULyraSettingScreen::CreateRegistry()
{
	ULyraGameSettingRegistry* NewRegistry = NewObject<ULyraGameSettingRegistry>();

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer()) { NewRegistry->Initialize(LocalPlayer); }

	return NewRegistry;
}

void ULyraSettingScreen::HandleBackAction()
{
	if (AttemptToPopNavigation()) { return; }

	ApplyChanges();
	DeactivateWidget();
}

void ULyraSettingScreen::HandleApplyAction() { ApplyChanges(); }

void ULyraSettingScreen::HandleCancelChangesAction() { CancelChanges(); }

void ULyraSettingScreen::OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty)
{
	if (bSettingsDirty)
	{
		if (!GetActionBindings().Contains(ApplyHandle)) { AddActionBinding(ApplyHandle); }
		if (!GetActionBindings().Contains(CancelChangesHandle)) { AddActionBinding(CancelChangesHandle); }
		return;
	}
	
	RemoveActionBinding(ApplyHandle);
	RemoveActionBinding(CancelChangesHandle);
}
