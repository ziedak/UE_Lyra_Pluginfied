// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnhancedInputSubsystems.h"
#include "CustomSettings/LyraSettingKeyboardInput.h"
// #include "DataSource/GameSettingDataSource.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "RegistrySettings/LyraGameSettingRegistry.h"
// #include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
// #include "PlayerMappableInputConfig.h"
#include "EditCondition/WhenPlatformSupportsMouseAndKeyboard.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"

class ULocalPlayer;

#define LOCTEXT_NAMESPACE "Lyra"

UGameSettingCollection* ULyraGameSettingRegistry::InitializeMouseAndKeyboardSettings(ULocalPlayer* InLocalPlayer)
{
	const auto Screen = UGameSettingCollection::CreateCollection("MouseAndKeyboardCollection",
	                                                             LOCTEXT("MouseAndKeyboardCollection_Name",
	                                                                     "Mouse & Keyboard"));
	Screen->Initialize(InLocalPlayer);


	Screen->AddSetting(AddMouseSensitivitySettings());
	AddKeyBindingSettings(Screen,InLocalPlayer);

	return Screen;
}

UGameSettingCollection* ULyraGameSettingRegistry::AddMouseSensitivitySettings()
{
	const auto Sensitivity = UGameSettingCollection::CreateCollection("MouseSensitivityCollection",
	                                                                  LOCTEXT("MouseSensitivityCollection_Name",
	                                                                          "Sensitivity"));

	const auto Interval = TRange<double>(0, 10);
	constexpr auto Step = 0.01;

	Sensitivity->AddSetting(AddMouseSensitivityYawSetting(Interval, Step));
	Sensitivity->AddSetting(AddMouseSensitivityPitchSetting(Interval, Step));
	Sensitivity->AddSetting(AddMouseTargetingMultiplierSetting(Interval, Step));
	Sensitivity->AddSetting(AddInvertVerticalAxisSetting());
	Sensitivity->AddSetting(AddInvertHorizontalAxisSetting());
	return Sensitivity;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::AddMouseSensitivityYawSetting(
	const TRange<double>& Range, const double Step)
{
	const auto Setting = UGameSettingValueScalarDynamic::CreateSettings(
		"MouseSensitivityYaw",
		LOCTEXT("MouseSensitivityYaw_Name", "X-Axis Sensitivity"),
		LOCTEXT("MouseSensitivityYaw_Description",
		        "Sets the sensitivity of the mouse's horizontal (x) axis. "
		        "With higher settings the camera will move faster when looking left and right with the mouse."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityX),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityX),
		GetDefault<ULyraSettingsShared>()->GetMouseSensitivityX(),
		UGameSettingValueScalarDynamic::RawTwoDecimals,
		FWhenPlatformSupportsMouseAndKeyboard::Get());

	Setting->SetSourceRangeAndStep(Range, Step);
	Setting->SetMinimumLimit(Step);
	return Setting;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::AddMouseSensitivityPitchSetting(
	const TRange<double>& Range, const double Step)
{
	const auto Setting = UGameSettingValueScalarDynamic::CreateSettings(
		TEXT("MouseSensitivityPitch"),
		LOCTEXT("MouseSensitivityPitch_Name", "Y-Axis Sensitivity"),
		LOCTEXT("MouseSensitivityPitch_Description",
		        "Sets the sensitivity of the mouse's vertical (y) axis. With higher settings the camera will "
		        "move faster when looking up and down with the mouse."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityY),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityY),
		GetDefault<ULyraSettingsShared>()->GetMouseSensitivityY(),
		UGameSettingValueScalarDynamic::RawTwoDecimals,
		FWhenPlatformSupportsMouseAndKeyboard::Get());

	Setting->SetSourceRangeAndStep(Range, Step);
	Setting->SetMinimumLimit(Step);
	return Setting;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::AddMouseTargetingMultiplierSetting(
	const TRange<double>& Range, const double Step)
{
	const auto Setting = UGameSettingValueScalarDynamic::CreateSettings(
		"MouseTargetingMultiplier",
		LOCTEXT("MouseTargetingMultiplier_Name", "Targeting Sensitivity"),
		LOCTEXT("MouseTargetingMultiplier_Description",
		        "Sets the modifier for reducing mouse sensitivity when targeting. 100% will have no slow down "
		        "when targeting. Lower settings will have more slow down when targeting."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetTargetingMultiplier),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetTargetingMultiplier),
		GetDefault<ULyraSettingsShared>()->GetTargetingMultiplier(),
		UGameSettingValueScalarDynamic::RawTwoDecimals,
		FWhenPlatformSupportsMouseAndKeyboard::Get());

	Setting->SetSourceRangeAndStep(Range, Step);
	Setting->SetMinimumLimit(Step);

	return Setting;
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::AddInvertVerticalAxisSetting()
{
	const auto Setting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		"InvertVerticalAxis",
		LOCTEXT("InvertVerticalAxis_Name", "Invert Vertical Axis"),
		LOCTEXT("InvertVerticalAxis_Description",
		        "Enable the inversion of the vertical look axis."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis),
		GetDefault<ULyraSettingsShared>()->GetInvertVerticalAxis());

	Setting->AddEditCondition(FWhenPlatformSupportsMouseAndKeyboard::Get());
	return Setting;
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::AddInvertHorizontalAxisSetting()
{
	const auto Setting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		"InvertHorizontalAxis",
		LOCTEXT("InvertHorizontalAxis_Name", "Invert Horizontal Axis"),
		LOCTEXT("InvertHorizontalAxis_Description",
		        "Enable the inversion of the Horizontal look axis."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis),
		GetDefault<ULyraSettingsShared>()->GetInvertHorizontalAxis());

	Setting->AddEditCondition(FWhenPlatformSupportsMouseAndKeyboard::Get());
	return Setting;
}

// Bindings for Mouse & Keyboard - Automatically Generated
void ULyraGameSettingRegistry::AddKeyBindingSettings(UGameSettingCollection* Screen, const ULocalPlayer* InLocalPlayer)
{
	const auto KeyBinding = UGameSettingCollection::CreateCollection("KeyBindingCollection",
	                                                                 LOCTEXT("KeyBindingCollection_Name",
	                                                                         "Keyboard & Mouse"));
	Screen->AddSetting(KeyBinding);

	const auto EiSubsystem = InLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	const auto UserSettings = EiSubsystem->GetUserSettings();

	// If you want to just get one profile pair, then you can do UserSettings->GetCurrentProfile
	// A map of key bindings mapped to their display category
	TMap<FString, UGameSettingCollection*> CategoryToSettingCollection;

	// Returns an existing setting collection for the display category if there is one.
	// If there isn't one, then it will create a new one and initialize it
	auto GetOrCreateSettingCollection = [&CategoryToSettingCollection, &Screen](
		FText DisplayCategory) -> UGameSettingCollection* {
		static const FString DefaultDevName = "Default_KBM";
		static const FText DefaultDevDisplayName = NSLOCTEXT("LyraInputSettings", "LyraInputDefaults",
		                                                     "Default Experiences");

		if (DisplayCategory.IsEmpty()) DisplayCategory = DefaultDevDisplayName;
		const FString DisplayCatString = DisplayCategory.ToString();

		// If we have already created a setting collection for this category, then return it
		if (UGameSettingCollection** ExistingCategory = CategoryToSettingCollection.Find(DisplayCatString))
			return *ExistingCategory;

		// Otherwise, create a new setting collection and add it to the screen
		UGameSettingCollection* ConfigSettingCollection = NewObject<UGameSettingCollection>();
		ConfigSettingCollection->SetDevName(FName(DisplayCatString));
		ConfigSettingCollection->SetDisplayName(DisplayCategory);
		Screen->AddSetting(ConfigSettingCollection);
		CategoryToSettingCollection.Add(DisplayCatString, ConfigSettingCollection);

		return ConfigSettingCollection;
	};

	static TSet<FName> CreatedMappingNames;
	CreatedMappingNames.Reset();

	for (const TPair<FGameplayTag, TObjectPtr<UEnhancedPlayerMappableKeyProfile>>& ProfilePair : UserSettings->
	     GetAllSavedKeyProfiles())
	{
		//const FGameplayTag& ProfileName = ProfilePair.Key;
		const TObjectPtr<UEnhancedPlayerMappableKeyProfile>& Profile = ProfilePair.Value;

		for (const TPair<FName, FKeyMappingRow>& RowPair : Profile->GetPlayerMappingRows())
		{
			// Create a setting row for anything with valid mappings and that we haven't created yet
			if (!RowPair.Value.HasAnyMappings()) continue;

			// We only want keyboard keys on this settings screen, so we will filter down by mappings
			// that are set to keyboard keys
			FPlayerMappableKeyQueryOptions Options = {};
			Options.KeyToMatch = EKeys::W;
			Options.bMatchBasicKeyTypes = true;

			const FText& DesiredDisplayCategory = RowPair.Value.Mappings.begin()->GetDisplayCategory();
			UGameSettingCollection* Collection = GetOrCreateSettingCollection(DesiredDisplayCategory);
			if (!Collection)
			{
				ensure(false);
				continue;
			}
			// Create the settings widget and initialize it, adding it to this config's section
			const auto InputBinding = NewObject<ULyraSettingKeyboardInput>();

			InputBinding->InitializeInputData(Profile, RowPair.Value, Options);
			InputBinding->AddEditCondition(FWhenPlatformSupportsMouseAndKeyboard::Get());

			Collection->AddSetting(InputBinding);
			CreatedMappingNames.Add(RowPair.Key);
		}
	}
}


#undef LOCTEXT_NAMESPACE
