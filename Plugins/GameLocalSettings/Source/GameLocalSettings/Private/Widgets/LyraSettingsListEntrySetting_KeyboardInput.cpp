// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/LyraSettingsListEntrySetting_KeyboardInput.h"

#include "CommonButtonBase.h"
#include "CommonUIExtensions.h"
#include "NativeGameplayTags.h"
#include "CustomSettings/LyraSettingKeyboardInput.h"
#include "Interfaces/IButtonInterface.h"
#include "Widgets/Misc/GameSettingPressAnyKey.h"
#include "Widgets/Misc/KeyAlreadyBoundWarning.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSettingsListEntrySetting_KeyboardInput)

#define LOCTEXT_NAMESPACE "LyraSettings"

UE_DEFINE_GAMEPLAY_TAG_STATIC(PressAnyKeyLayer, "UI.Layer.Modal");

void ULyraSettingsListEntrySetting_KeyboardInput::SetSetting(UGameSetting* InSetting)
{
	KeyboardInputSetting = CastChecked<ULyraSettingKeyboardInput>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}

void ULyraSettingsListEntrySetting_KeyboardInput::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_PrimaryKey->OnClicked().AddUObject(this, &ThisClass::HandlePrimaryKeyClicked);
	Button_SecondaryKey->OnClicked().AddUObject(this, &ThisClass::HandleSecondaryKeyClicked);
	Button_Clear->OnClicked().AddUObject(this, &ThisClass::HandleClearClicked);
	Button_ResetToDefault->OnClicked().AddUObject(this, &ThisClass::HandleResetToDefaultClicked);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandlePrimaryKeyClicked()
{
	UGameSettingPressAnyKey* PressAnyKeyPanel = CastChecked<UGameSettingPressAnyKey>(
		UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(),
		                                                  PressAnyKeyLayer,
		                                                  PressAnyKeyPanelClass));
	PressAnyKeyPanel->OnKeySelected.AddUObject(this, &ThisClass::HandlePrimaryKeySelected, PressAnyKeyPanel);
	PressAnyKeyPanel->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, PressAnyKeyPanel);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandleSecondaryKeyClicked()
{
	UGameSettingPressAnyKey* PressAnyKeyPanel = CastChecked<UGameSettingPressAnyKey>(
		UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), PressAnyKeyLayer,
		                                                  PressAnyKeyPanelClass));
	PressAnyKeyPanel->OnKeySelected.AddUObject(this, &ThisClass::HandleSecondaryKeySelected, PressAnyKeyPanel);
	PressAnyKeyPanel->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, PressAnyKeyPanel);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandlePrimaryKeySelected(
	const FKey& InKey, UGameSettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	ChangeBinding(0, InKey);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandleSecondaryKeySelected(
	const FKey& InKey, UGameSettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	ChangeBinding(1, InKey);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandlePrimaryDuplicateKeySelected(
	const FKey& InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const
{
	DuplicateKeyPressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	KeyboardInputSetting->ChangeBinding(0, OriginalKeyToBind);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandleSecondaryDuplicateKeySelected(
	const FKey& InKey, UKeyAlreadyBoundWarning* DuplicateKeyPressAnyKeyPanel) const
{
	DuplicateKeyPressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	KeyboardInputSetting->ChangeBinding(1, OriginalKeyToBind);
}

void ULyraSettingsListEntrySetting_KeyboardInput::ChangeBinding(const int32 InKeyBindSlot, const FKey& InKey)
{
	OriginalKeyToBind = InKey;
	TArray<FName> ActionsForKey;
	KeyboardInputSetting->GetAllMappedActionsFromKey(InKeyBindSlot, InKey, ActionsForKey);
	if (ActionsForKey.IsEmpty())
	{
		KeyboardInputSetting->ChangeBinding(InKeyBindSlot, InKey);
		return;
	}

	UKeyAlreadyBoundWarning* KeyAlreadyBoundWarning = CastChecked<UKeyAlreadyBoundWarning>(
		UCommonUIExtensions::PushContentToLayer_ForPlayer(GetOwningLocalPlayer(), PressAnyKeyLayer,
		                                                  KeyAlreadyBoundWarningPanelClass));

	FString ActionNames;
	for (FName ActionName : ActionsForKey) { ActionNames += ActionName.ToString() += ", "; }

	FFormatNamedArguments Args;
	Args.Add(TEXT("InKey"), InKey.GetDisplayName());
	Args.Add(TEXT("ActionNames"), FText::FromString(ActionNames));

	KeyAlreadyBoundWarning->SetWarningText(FText::Format(
		LOCTEXT("WarningText", "{InKey} is already bound to {ActionNames} are you sure you want to rebind it?"),
		Args));
	KeyAlreadyBoundWarning->SetCancelText(FText::Format(
		LOCTEXT("CancelText", "Press escape to cancel, or press {InKey} again to confirm rebinding."), Args));

	KeyAlreadyBoundWarning->OnKeySelected.AddUObject(this,
	                                                 InKeyBindSlot == 1
		                                                 ? &ThisClass::HandleSecondaryDuplicateKeySelected
		                                                 : &ThisClass::HandlePrimaryDuplicateKeySelected,
	                                                 KeyAlreadyBoundWarning);

	KeyAlreadyBoundWarning->OnKeySelectionCanceled.AddUObject(this,
	                                                          &ThisClass::HandleKeySelectionCanceled,
	                                                          KeyAlreadyBoundWarning);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandleKeySelectionCanceled(
	UGameSettingPressAnyKey* PressAnyKeyPanel) const { PressAnyKeyPanel->OnKeySelectionCanceled.RemoveAll(this); }

void ULyraSettingsListEntrySetting_KeyboardInput::HandleKeySelectionCanceled(
	UKeyAlreadyBoundWarning* PressAnyKeyPanel) const { PressAnyKeyPanel->OnKeySelectionCanceled.RemoveAll(this); }

void ULyraSettingsListEntrySetting_KeyboardInput::HandleClearClicked() const
{
	KeyboardInputSetting->ChangeBinding(0, EKeys::Invalid);
	KeyboardInputSetting->ChangeBinding(1, EKeys::Invalid);
}

void ULyraSettingsListEntrySetting_KeyboardInput::HandleResetToDefaultClicked() const
{
	KeyboardInputSetting->ResetToDefault();
}

void ULyraSettingsListEntrySetting_KeyboardInput::OnSettingChanged() { Refresh(); }

void ULyraSettingsListEntrySetting_KeyboardInput::Refresh() const
{
	if (!(ensure(KeyboardInputSetting))) { return; }

	if (Button_PrimaryKey && Button_PrimaryKey->Implements<UButtonInterface>())
	{
		IButtonInterface::Execute_SetButtonText(Button_PrimaryKey,
		                                        KeyboardInputSetting->GetKeyTextFromSlot(
			                                        EPlayerMappableKeySlot::First));
		// if (const auto IButton_PrimaryKey = Cast<IButtonInterface>(Button_PrimaryKey))
		// {
		// 	//IButton_PrimaryKey->SetButtonText(KeyboardInputSetting->GetKeyTextFromSlot(EPlayerMappableKeySlot::First));
		// }
	}
	else { UE_LOG(LogTemp, Error, TEXT("Button_PrimaryKey does not Implement IButtonInterface")); }

	if (Button_SecondaryKey && Button_SecondaryKey->Implements<UButtonInterface>())
	{
		IButtonInterface::Execute_SetButtonText(Button_SecondaryKey,
		                                        KeyboardInputSetting->GetKeyTextFromSlot(
			                                        EPlayerMappableKeySlot::Second));

		// if (const auto IButton_SecondaryKey = Cast<IButtonInterface>(Button_SecondaryKey))
		// {
		// 	IButton_SecondaryKey->
		// 		SetButtonText(KeyboardInputSetting->GetKeyTextFromSlot(EPlayerMappableKeySlot::First));
		// }
	}
	else { UE_LOG(LogTemp, Error, TEXT("Button_SecondaryKey does not Implement IButtonInterface")); }
	// Button_PrimaryKey->SetButtonText(KeyboardInputSetting->GetKeyTextFromSlot(EPlayerMappableKeySlot::Second));
	// Button_SecondaryKey->SetButtonText(KeyboardInputSetting->GetKeyTextFromSlot(EPlayerMappableKeySlot::Second));

	// Only display the reset to default button if a mapping is customized
	if (!(ensure(Button_ResetToDefault))) { return; }

	if (KeyboardInputSetting->IsMappingCustomized())
	{
		Button_ResetToDefault->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	Button_ResetToDefault->SetVisibility(ESlateVisibility::Hidden);
}

void ULyraSettingsListEntrySetting_KeyboardInput::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	KeyboardInputSetting = nullptr;
}

#undef LOCTEXT_NAMESPACE
