// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomSettings/LyraSettingKeyboardInput.h"

#include "EnhancedInputSubsystems.h"
#include "PlayerMappableInputConfig.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSettingKeyboardInput)

class ULocalPlayer;

#define LOCTEXT_NAMESPACE "LyraSettings"

namespace Lyra::ErrorMessages
{
	static const FText UnknownMappingName = LOCTEXT("LyraErrors_UnknownMappingName", "Unknown Mapping");
}


ULyraSettingKeyboardInput::ULyraSettingKeyboardInput()
{
	bReportAnalytics = false;
}

FText ULyraSettingKeyboardInput::GetSettingDisplayName() const
{
	const FKeyMappingRow* Row = FindKeyMappingRow();
	if (Row && Row->HasAnyMappings())
		return Row->Mappings.begin()->GetDisplayName();

	return Lyra::ErrorMessages::UnknownMappingName;
}

FText ULyraSettingKeyboardInput::GetSettingDisplayCategory() const
{
	const auto Row = FindKeyMappingRow();
	if (Row && Row->HasAnyMappings())
		return Row->Mappings.begin()->GetDisplayCategory();

	return Lyra::ErrorMessages::UnknownMappingName;
}

const FKeyMappingRow* ULyraSettingKeyboardInput::FindKeyMappingRow() const
{
	if (const auto Profile = FindMappableKeyProfile())
		return Profile->FindKeyMappingRow(ActionMappingName);

	ensure(false);
	return nullptr;
}

UEnhancedPlayerMappableKeyProfile* ULyraSettingKeyboardInput::FindMappableKeyProfile() const
{
	if (const auto Settings = GetUserSettings())
		return Settings->GetKeyProfileWithIdentifier(ProfileIdentifier);

	ensure(false);
	return nullptr;
}

UEnhancedInputUserSettings* ULyraSettingKeyboardInput::GetUserSettings() const
{
	// Map the key to the player key profile
	if (const auto System = LocalPlayer->GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>())
		return System->GetUserSettings();

	return nullptr;
}

void ULyraSettingKeyboardInput::OnInitialized()
{
	DynamicDetails = FGetGameSettingsDetails::CreateLambda([this](ULocalPlayer&)
		{
			const auto Row = FindKeyMappingRow();
			if (Row && Row->HasAnyMappings())
				return FText::Format(
					LOCTEXT("DynamicDetails_KeyboardInputAction", "Bindings for {0}"),
					Row->Mappings.begin()->GetDisplayName());

			return FText::GetEmpty();
		}

	);

	Super::OnInitialized();
}

void ULyraSettingKeyboardInput::InitializeInputData(const UEnhancedPlayerMappableKeyProfile* KeyProfile,
                                                    const FKeyMappingRow& MappingData,
                                                    const FPlayerMappableKeyQueryOptions& InQueryOptions)
{
	check(KeyProfile);

	ProfileIdentifier = KeyProfile->GetProfileIdentifer();
	QueryOptions = InQueryOptions;

	for (const FPlayerKeyMapping& Mapping : MappingData.Mappings)
	{
		// Only add mappings that pass the query filters that have been provided upon creation
		if (!KeyProfile->DoesMappingPassQueryOptions(Mapping, QueryOptions)) continue;

		ActionMappingName = Mapping.GetMappingName();
		InitialKeyMappings.Add(Mapping.GetSlot(), Mapping.GetCurrentKey());
		const FText& MappingDisplayName = Mapping.GetDisplayName();

		if (!MappingDisplayName.IsEmpty())
			SetDisplayName(MappingDisplayName);
	}

	const FString NameString = TEXT("KBM_Input_") + ActionMappingName.ToString();
	SetDevName(*NameString);
}

FText ULyraSettingKeyboardInput::GetKeyTextFromSlot(const EPlayerMappableKeySlot InSlot) const
{
	const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile();
	if (!Profile)
		return EKeys::Invalid.GetDisplayName();

	FPlayerMappableKeyQueryOptions QueryOptionsForSlot = QueryOptions;
	QueryOptionsForSlot.SlotToMatch = InSlot;

	if (const FKeyMappingRow* Row = FindKeyMappingRow())
	{
		for (const FPlayerKeyMapping& Mapping : Row->Mappings)
		{
			if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptionsForSlot))
				return Mapping.GetCurrentKey().GetDisplayName();
		}
	}

	return EKeys::Invalid.GetDisplayName();
}


PRAGMA_DISABLE_DEPRECATION_WARNINGS

FText ULyraSettingKeyboardInput::GetPrimaryKeyText() const
{
	return GetKeyTextFromSlot(EPlayerMappableKeySlot::First);
}

FText ULyraSettingKeyboardInput::GetSecondaryKeyText() const
{
	return GetKeyTextFromSlot(EPlayerMappableKeySlot::Second);
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS

void ULyraSettingKeyboardInput::ResetToDefault()
{
	if (UEnhancedInputUserSettings* Settings = GetUserSettings())
	{
		FMapPlayerKeyArgs Args = {};
		Args.MappingName = ActionMappingName;

		FGameplayTagContainer FailureReason;
		Settings->ResetAllPlayerKeysInRow(Args, FailureReason);

		NotifySettingChanged(EGameSettingChangeReason::Change);
	}
}

void ULyraSettingKeyboardInput::StoreInitial()
{
	const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile();
	if (!Profile) return;

	if (const FKeyMappingRow* Row = FindKeyMappingRow())
	{
		for (const FPlayerKeyMapping& Mapping : Row->Mappings)
		{
			if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptions))
			{
				ActionMappingName = Mapping.GetMappingName();
				InitialKeyMappings.Add(Mapping.GetSlot(), Mapping.GetCurrentKey());
			}
		}
	}
}

void ULyraSettingKeyboardInput::RestoreToInitial()
{
	for (TPair<EPlayerMappableKeySlot, FKey> Pair : InitialKeyMappings)
	{
		ChangeBinding(static_cast<int32>(Pair.Key), Pair.Value);
	}
}

bool ULyraSettingKeyboardInput::ChangeBinding(const int32 InKeyBindSlot, const FKey& NewKey)
{
	if (NewKey.IsGamepadKey()) return false;


	FMapPlayerKeyArgs Args = {};
	Args.MappingName = ActionMappingName;
	Args.Slot = static_cast<EPlayerMappableKeySlot>(static_cast<uint8>(InKeyBindSlot));
	Args.NewKey = NewKey;
	// If you want to, you can additionally specify this mapping to only be applied to a certain hardware device or key profile
	//Args.ProfileId =
	//Args.HardwareDeviceId =

	if (UEnhancedInputUserSettings* Settings = GetUserSettings())
	{
		FGameplayTagContainer FailureReason;
		Settings->MapPlayerKey(Args, FailureReason);
		NotifySettingChanged(EGameSettingChangeReason::Change);
	}

	return true;
}

void ULyraSettingKeyboardInput::GetAllMappedActionsFromKey(int32 InKeyBindSlot, const FKey& Key,
                                                           TArray<FName>& OutActionNames) const
{
	if (const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile())
		Profile->GetMappingNamesForKey(Key, OutActionNames);
}

bool ULyraSettingKeyboardInput::IsMappingCustomized() const
{
	const UEnhancedPlayerMappableKeyProfile* Profile = FindMappableKeyProfile();
	const FKeyMappingRow* Row = FindKeyMappingRow();
	if (!Profile || !Row) return false;

	for (const FPlayerKeyMapping& Mapping : Row->Mappings)
	{
		if (Profile->DoesMappingPassQueryOptions(Mapping, QueryOptions))
		{
			if (Mapping.IsCustomized()) return true;
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
