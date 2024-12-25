// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonInputBaseTypes.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "RegistrySettings/LyraGameSettingRegistry.h"
#include "Settings/LyraSettingsLocal.h"
#include "Settings/LyraSettingsShared.h"
#include "NativeGameplayTags.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"

#define LOCTEXT_NAMESPACE "Lyra"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_SupportsGamepad, "Platform.Trait.Input.SupportsGamepad");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Platform_Trait_Input_SupportsTriggerHaptics,
                              "Platform.Trait.Input.SupportsTriggerHaptics");

UGameSettingCollection* ULyraGameSettingRegistry::InitializeGamepadSettings(ULocalPlayer* InLocalPlayer)
{
	const auto Screen = UGameSettingCollection::CreateCollection(
		TEXT("GamepadCollection"), LOCTEXT("GamepadCollection_Name", "Gamepad"));
	Screen->Initialize(InLocalPlayer);
	Screen->AddSetting(CreateHardwareCollection());
	Screen->AddSetting(CreateGamepadBindingCollection());
	Screen->AddSetting(CreateBasicSensitivityCollection());
	Screen->AddSetting(CreateDeadZoneCollection());

	return Screen;
}


UGameSettingCollection* ULyraGameSettingRegistry::CreateHardwareCollection()
{
	const auto Hardware = UGameSettingCollection::CreateCollection(
		TEXT("HardwareCollection"), LOCTEXT("HardwareCollection_Name", "Hardware"));

	Hardware->AddSetting(CreateControllerHardwareSetting());
	Hardware->AddSetting(CreateGamepadVibrationSetting());
	Hardware->AddSetting(CreateInvertVerticalAxisSetting());
	Hardware->AddSetting(CreateInvertHorizontalAxisSetting());

	return Hardware;
}

UGameSettingValueDiscreteDynamic* ULyraGameSettingRegistry::CreateControllerHardwareSetting()
{
	UGameSettingValueDiscreteDynamic* Setting = NewObject<UGameSettingValueDiscreteDynamic>();
	Setting->SetDevName(TEXT("ControllerHardware"));
	Setting->SetDisplayName(LOCTEXT("ControllerHardware_Name", "Controller Hardware"));
	Setting->SetDescriptionRichText(LOCTEXT("ControllerHardware_Description", "The type of controller you're using."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetControllerPlatform));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetControllerPlatform));

	if (UCommonInputPlatformSettings* PlatformInputSettings = UPlatformSettingsManager::Get().GetSettingsForPlatform<
		UCommonInputPlatformSettings>())
	{
		const TArray<TSoftClassPtr<UCommonInputBaseControllerData>>& ControllerDatas = PlatformInputSettings->
			GetControllerData();
		for (TSoftClassPtr<UCommonInputBaseControllerData> ControllerDataPtr : ControllerDatas)
		{
			if (TSubclassOf<UCommonInputBaseControllerData> ControllerDataClass = ControllerDataPtr.LoadSynchronous())
			{
				const UCommonInputBaseControllerData* ControllerData = ControllerDataClass.GetDefaultObject();
				if (ControllerData->InputType == ECommonInputType::Gamepad)
				{
					Setting->AddDynamicOption(ControllerData->GamepadName.ToString(),
					                          ControllerData->GamepadDisplayName);
				}
			}
		}
		// The setting if we can select more than one game controller type on this platform
		// and we are allowed to change it
		if (Setting->GetDynamicOptions().Num() > 1 && PlatformInputSettings->CanChangeGamepadType())
		{
			const FName CurrentControllerPlatform = GetDefault<ULyraSettingsLocal>()->GetControllerPlatform();
			if (CurrentControllerPlatform == NAME_None)
			{
				Setting->SetDiscreteOptionByIndex(0);
			}
			else
			{
				Setting->SetDefaultValueFromString(CurrentControllerPlatform.ToString());
			}
		}
	}

	return Setting;
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::CreateGamepadVibrationSetting()
{
	return UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		TEXT("GamepadVibration"),
		LOCTEXT("GamepadVibration_Name", "Vibration"),
		LOCTEXT("GamepadVibration_Description", "Turns controller vibration on/off."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetForceFeedbackEnabled),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetForceFeedbackEnabled),
		GetDefault<ULyraSettingsShared>()->GetForceFeedbackEnabled()
	);
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::CreateInvertVerticalAxisSetting()
{
	return UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		TEXT("InvertVerticalAxis_Gamepad"),
		LOCTEXT("InvertVerticalAxis_Gamepad_Name", "Invert Vertical Axis"),
		LOCTEXT("InvertVerticalAxis_Gamepad_Description", "Enable the inversion of the vertical look axis."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis),
		GetDefault<ULyraSettingsShared>()->GetInvertVerticalAxis()
	);
}

UGameSettingValueDiscreteDynamic_Bool* ULyraGameSettingRegistry::CreateInvertHorizontalAxisSetting()
{
	return UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
		TEXT("InvertHorizontalAxis_Gamepad"),
		LOCTEXT("InvertHorizontalAxis_Gamepad_Name", "Invert Horizontal Axis"),
		LOCTEXT("InvertHorizontalAxis_Gamepad_Description", "Enable the inversion of the Horizontal look axis."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis),
		GetDefault<ULyraSettingsShared>()->GetInvertHorizontalAxis()
	);
}

UGameSettingCollection* ULyraGameSettingRegistry::CreateGamepadBindingCollection()
{
	return UGameSettingCollection::CreateCollection(
		TEXT("GamepadBindingCollection"), LOCTEXT("GamepadBindingCollection_Name", "Controls"));
}

UGameSettingCollection* ULyraGameSettingRegistry::CreateBasicSensitivityCollection()
{
	const auto BasicSensitivity = UGameSettingCollection::CreateCollection(
		TEXT("BasicSensitivityCollection"), LOCTEXT("BasicSensitivityCollection_Name", "Sensitivity"));

	BasicSensitivity->AddSetting(CreateLookSensitivityPresetSetting());
	BasicSensitivity->AddSetting(CreateLookSensitivityPresetAdsSetting());

	return BasicSensitivity;
}

UGameSettingValueDiscreteDynamic_Enum* ULyraGameSettingRegistry::CreateLookSensitivityPresetSetting()
{
	const TMap<EGamepadSensitivity, FText> EGamepadSensitivityText = {
		{EGamepadSensitivity::Slow, LOCTEXT("EFortGamepadSensitivity_Slow", "1 (Slow)")},
		{EGamepadSensitivity::SlowPlus, LOCTEXT("EFortGamepadSensitivity_SlowPlus", "2 (Slow+)")},
		{EGamepadSensitivity::SlowPlusPlus, LOCTEXT("EFortGamepadSensitivity_SlowPlusPlus", "3 (Slow++)")},
		{EGamepadSensitivity::Normal, LOCTEXT("EFortGamepadSensitivity_Normal", "4 (Normal)")},
		{EGamepadSensitivity::NormalPlus, LOCTEXT("EFortGamepadSensitivity_NormalPlus", "5 (Normal+)")},
		{EGamepadSensitivity::NormalPlusPlus, LOCTEXT("EFortGamepadSensitivity_NormalPlusPlus", "6 (Normal++)")},
		{EGamepadSensitivity::Fast, LOCTEXT("EFortGamepadSensitivity_Fast", "7 (Fast)")},
		{EGamepadSensitivity::FastPlus, LOCTEXT("EFortGamepadSensitivity_FastPlus", "8 (Fast+)")},
		{EGamepadSensitivity::FastPlusPlus, LOCTEXT("EFortGamepadSensitivity_FastPlusPlus", "9 (Fast++)")},
		{EGamepadSensitivity::Insane, LOCTEXT("EFortGamepadSensitivity_Insane", "10 (Insane)")}
	};

	return UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings<EGamepadSensitivity>(
		TEXT("LookSensitivityPreset"),
		LOCTEXT("LookSensitivityPreset_Name", "Look Sensitivity"),
		LOCTEXT("LookSensitivityPreset_Description", "How quickly your view rotates."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadLookSensitivityPreset),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetLookSensitivityPreset),
		GetDefault<ULyraSettingsShared>()->GetGamepadLookSensitivityPreset(),
		EGamepadSensitivityText
	);
}

UGameSettingValueDiscreteDynamic_Enum* ULyraGameSettingRegistry::CreateLookSensitivityPresetAdsSetting()
{
	const TMap<EGamepadSensitivity, FText> EGamepadSensitivityText = {
		{EGamepadSensitivity::Slow, LOCTEXT("EFortGamepadSensitivity_Slow", "1 (Slow)")},
		{EGamepadSensitivity::SlowPlus, LOCTEXT("EFortGamepadSensitivity_SlowPlus", "2 (Slow+)")},
		{EGamepadSensitivity::SlowPlusPlus, LOCTEXT("EFortGamepadSensitivity_SlowPlusPlus", "3 (Slow++)")},
		{EGamepadSensitivity::Normal, LOCTEXT("EFortGamepadSensitivity_Normal", "4 (Normal)")},
		{EGamepadSensitivity::NormalPlus, LOCTEXT("EFortGamepadSensitivity_NormalPlus", "5 (Normal+)")},
		{EGamepadSensitivity::NormalPlusPlus, LOCTEXT("EFortGamepadSensitivity_NormalPlusPlus", "6 (Normal++)")},
		{EGamepadSensitivity::Fast, LOCTEXT("EFortGamepadSensitivity_Fast", "7 (Fast)")},
		{EGamepadSensitivity::FastPlus, LOCTEXT("EFortGamepadSensitivity_FastPlus", "8 (Fast+)")},
		{EGamepadSensitivity::FastPlusPlus, LOCTEXT("EFortGamepadSensitivity_FastPlusPlus", "9 (Fast++)")},
		{EGamepadSensitivity::Insane, LOCTEXT("EFortGamepadSensitivity_Insane", "10 (Insane)")}
	};

	return UGameSettingValueDiscreteDynamic_Enum::CreateEnumSettings<EGamepadSensitivity>(
		TEXT("LookSensitivityPresetAds"),
		LOCTEXT("LookSensitivityPresetAds_Name", "Aim Sensitivity (ADS)"),
		LOCTEXT("LookSensitivityPresetAds_Description",
		        "How quickly your view rotates while aiming down sights (ADS)."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadTargetingSensitivityPreset),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadTargetingSensitivityPreset),
		GetDefault<ULyraSettingsShared>()->GetGamepadTargetingSensitivityPreset(),
		EGamepadSensitivityText
	);
}

UGameSettingCollection* ULyraGameSettingRegistry::CreateDeadZoneCollection()
{
	constexpr float GamepadSensitivityMinimumLimit(0.05);
	constexpr float GamepadSensitivityMaximumLimit(0.95);
	const auto DeadZone = UGameSettingCollection::CreateCollection(
		TEXT("DeadZoneCollection"), LOCTEXT("DeadZoneCollection_Name", "Controller DeadZone"));

	DeadZone->AddSetting(
		CreateMoveStickDeadZoneSetting(GamepadSensitivityMinimumLimit, GamepadSensitivityMaximumLimit));
	DeadZone->AddSetting(
		CreateLookStickDeadZoneSetting(GamepadSensitivityMinimumLimit, GamepadSensitivityMaximumLimit));

	return DeadZone;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::CreateMoveStickDeadZoneSetting(float MinLimit, float MaxLimit)
{
	const auto Setting = UGameSettingValueScalarDynamic::CreateSettings(
		TEXT("MoveStickDeadZone"),
		LOCTEXT("MoveStickDeadZone_Name", "Left Stick DeadZone"),
		LOCTEXT("MoveStickDeadZone_Description",
		        "Increase or decrease the area surrounding the stick that we ignore input from. Setting this value too low may result in the character continuing to move even after removing your finger from the stick."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadMoveStickDeadZone),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadMoveStickDeadZone),
		GetDefault<ULyraSettingsShared>()->GetGamepadMoveStickDeadZone(),
		UGameSettingValueScalarDynamic::ZeroToOnePercent,
		FWhenPlayingAsPrimaryPlayer::Get()
	);

	Setting->SetMinimumLimit(MinLimit);
	Setting->SetMaximumLimit(MaxLimit);

	return Setting;
}

UGameSettingValueScalarDynamic* ULyraGameSettingRegistry::CreateLookStickDeadZoneSetting(float MinLimit, float MaxLimit)
{
	const auto Setting = UGameSettingValueScalarDynamic::CreateSettings(
		TEXT("LookStickDeadZone"),
		LOCTEXT("LookStickDeadZone_Name", "Right Stick DeadZone"),
		LOCTEXT("LookStickDeadZone_Description",
		        "Increase or decrease the area surrounding the stick that we ignore input from. Setting this value too low may result in the camera continuing to move even after removing your finger from the stick."),
		GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadLookStickDeadZone),
		GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadLookStickDeadZone),
		GetDefault<ULyraSettingsShared>()->GetGamepadLookStickDeadZone(),
		UGameSettingValueScalarDynamic::ZeroToOnePercent,
		FWhenPlayingAsPrimaryPlayer::Get()
	);

	Setting->SetMinimumLimit(MinLimit);
	Setting->SetMaximumLimit(MaxLimit);

	return Setting;
}


// UGameSettingCollection* ULyraGameSettingRegistry::InitializeGamepadSettings(ULocalPlayer* InLocalPlayer)
// {
// 	const auto Screen = UGameSettingCollection::CreateCollection(
// 		TEXT("GamepadCollection"), LOCTEXT("GamepadCollection_Name", "Gamepad"));
// 	Screen->Initialize(InLocalPlayer);
//
// 	// Hardware
// 	{
// 		const auto Hardware = UGameSettingCollection::CreateCollection(
// 			TEXT("HardwareCollection"),LOCTEXT("HardwareCollection_Name", "Hardware"));
// 		Screen->AddSetting(Hardware);
//
// 		//----------------------------------------------------------------------------------
// 		{
// 			UGameSettingValueDiscreteDynamic* Setting = NewObject<UGameSettingValueDiscreteDynamic>();
// 			Setting->SetDevName(TEXT("ControllerHardware"));
// 			Setting->SetDisplayName(LOCTEXT("ControllerHardware_Name", "Controller Hardware"));
// 			Setting->SetDescriptionRichText(LOCTEXT("ControllerHardware_Description",
// 			                                        "The type of controller you're using."));
// 			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetControllerPlatform));
// 			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetControllerPlatform));
//
// 			if (UCommonInputPlatformSettings* PlatformInputSettings = UPlatformSettingsManager::Get().
// 				GetSettingsForPlatform<UCommonInputPlatformSettings>())
// 			{
// 				const TArray<TSoftClassPtr<UCommonInputBaseControllerData>>& ControllerDatas = PlatformInputSettings->
// 					GetControllerData();
// 				for (TSoftClassPtr<UCommonInputBaseControllerData> ControllerDataPtr : ControllerDatas)
// 				{
// 					if (TSubclassOf<UCommonInputBaseControllerData> ControllerDataClass = ControllerDataPtr.
// 						LoadSynchronous())
// 					{
// 						const UCommonInputBaseControllerData* ControllerData = ControllerDataClass.GetDefaultObject();
// 						if (ControllerData->InputType == ECommonInputType::Gamepad)
// 						{
// 							Setting->AddDynamicOption(ControllerData->GamepadName.ToString(),
// 							                          ControllerData->GamepadDisplayName);
// 						}
// 					}
// 				}
//
// 				// Add the setting if we can select more than one game controller type on this platform
// 				// and we are allowed to change it
// 				if (Setting->GetDynamicOptions().Num() > 1 && PlatformInputSettings->CanChangeGamepadType())
// 				{
// 					Hardware->AddSetting(Setting);
//
// 					const FName CurrentControllerPlatform = GetDefault<ULyraSettingsLocal>()->GetControllerPlatform();
// 					if (CurrentControllerPlatform == NAME_None)
// 					{
// 						Setting->SetDiscreteOptionByIndex(0);
// 					}
// 					else
// 					{
// 						Setting->SetDefaultValueFromString(CurrentControllerPlatform.ToString());
// 					}
// 				}
// 			}
// 		}
// 		//----------------------------------------------------------------------------------
// 		{
// 			const auto Setting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
// 				TEXT("GamepadVibration"),
// 				LOCTEXT("GamepadVibration_Name", "Vibration"),
// 				LOCTEXT("GamepadVibration_Description", "Turns controller vibration on/off."),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(GetForceFeedbackEnabled),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(SetForceFeedbackEnabled),
// 				GetDefault<ULyraSettingsShared>()->GetForceFeedbackEnabled()
// 			);
//
// 			Hardware->AddSetting(Setting);
// 		}
// 		//----------------------------------------------------------------------------------
// 		{
// 			const auto Setting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
// 				TEXT("InvertVerticalAxis_Gamepad"),
// 				LOCTEXT("InvertVerticalAxis_Gamepad_Name", "Invert Vertical Axis"),
// 				LOCTEXT("InvertVerticalAxis_Gamepad_Description",
// 				        "Enable the inversion of the vertical look axis."),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis),
// 				GetDefault<ULyraSettingsShared>()->GetInvertVerticalAxis());
//
// 			Hardware->AddSetting(Setting);
// 		}
// 		//----------------------------------------------------------------------------------
// 		{
// 			const auto Setting = UGameSettingValueDiscreteDynamic_Bool::CreateSettings(
// 				TEXT("InvertHorizontalAxis_Gamepad"),
// 				LOCTEXT("InvertHorizontalAxis_Gamepad_Name", "Invert Horizontal Axis"),
// 				LOCTEXT("InvertHorizontalAxis_Gamepad_Description",
// 				        "Enable the inversion of the Horizontal look axis."),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis),
// 				GetDefault<ULyraSettingsShared>()->GetInvertHorizontalAxis());
//
// 			Hardware->AddSetting(Setting);
// 		}
// 		//----------------------------------------------------------------------------------
// 	}
//
// 	{
// 		const auto GamepadBinding = UGameSettingCollection::CreateCollection(
// 			TEXT("GamepadBindingCollection"),LOCTEXT("GamepadBindingCollection_Name", "Controls"));
// 		Screen->AddSetting(GamepadBinding);
// 	}
// 	// Basic - Look Sensitivity
// 	{
// 		const auto BasicSensitivity = UGameSettingCollection::CreateCollection(
// 			TEXT("BasicSensitivityCollection"),LOCTEXT("BasicSensitivityCollection_Name", "Sensitivity"));
// 		Screen->AddSetting(BasicSensitivity);
//
// 		//----------------------------------------------------------------------------------
//
//
// 		 const TMap<EGamepadSensitivity,  FText> EGamepadSensitivityText = {
// 			{EGamepadSensitivity::Slow,LOCTEXT("EFortGamepadSensitivity_Slow", "1 (Slow)")},
// 			{EGamepadSensitivity::SlowPlus,LOCTEXT("EFortGamepadSensitivity_SlowPlus", "2 (Slow+)")},
// 			{EGamepadSensitivity::SlowPlusPlus,LOCTEXT("EFortGamepadSensitivity_SlowPlusPlus", "3 (Slow++)")},
// 			{EGamepadSensitivity::Normal,LOCTEXT("EFortGamepadSensitivity_Normal", "4 (Normal)")},
// 			{EGamepadSensitivity::NormalPlus,LOCTEXT("EFortGamepadSensitivity_NormalPlus", "5 (Normal+)")},
// 			{EGamepadSensitivity::NormalPlusPlus,LOCTEXT("EFortGamepadSensitivity_NormalPlusPlus", "6 (Normal++)")},
// 			{EGamepadSensitivity::Fast,LOCTEXT("EFortGamepadSensitivity_Fast", "7 (Fast)")},
// 			{EGamepadSensitivity::FastPlus,LOCTEXT("EFortGamepadSensitivity_FastPlus", "8 (Fast+)")},
// 			{EGamepadSensitivity::FastPlusPlus,LOCTEXT("EFortGamepadSensitivity_FastPlusPlus", "9 (Fast++)")},
// 			{EGamepadSensitivity::Insane,LOCTEXT("EFortGamepadSensitivity_Insane", "10 (Insane)")},
// 		};
// 		{
// 			const auto Setting =UGameSettingValueDiscreteDynamic_Enum:: CreateEnumSettings<EGamepadSensitivity>(
// 				TEXT("LookSensitivityPreset"),
// 				LOCTEXT("LookSensitivityPreset_Name", "Look Sensitivity"),
// 				LOCTEXT("LookSensitivityPreset_Description", "How quickly your view rotates."),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadLookSensitivityPreset),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(SetLookSensitivityPreset),
// 				GetDefault<ULyraSettingsShared>()->GetGamepadLookSensitivityPreset(),
// 				EGamepadSensitivityText);
//
// 			BasicSensitivity->AddSetting(Setting);
// 		}
// 		//----------------------------------------------------------------------------------
// 		{
// 			auto Setting =UGameSettingValueDiscreteDynamic_Enum:: CreateEnumSettings<EGamepadSensitivity>(
// 				TEXT("LookSensitivityPresetAds"),
// 				LOCTEXT("LookSensitivityPresetAds_Name", "Aim Sensitivity (ADS)"),
// 				LOCTEXT("LookSensitivityPresetAds_Description",
// 				        "How quickly your view rotates while aiming down sights (ADS)."),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadTargetingSensitivityPreset),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadTargetingSensitivityPreset),
// 				GetDefault<ULyraSettingsShared>()->GetGamepadTargetingSensitivityPreset(),
// 				EGamepadSensitivityText);
//
// 			BasicSensitivity->AddSetting(Setting);
// 		}
// 		//----------------------------------------------------------------------------------
// 	}
//
// 	// Dead Zone
// 	{
// 		constexpr float GamepadSensitivityMinimumLimit(0.05);
// 		constexpr float GamepadSensitivityMaximumLimit(0.95);
// 		const auto DeadZone = UGameSettingCollection::CreateCollection(
// 			TEXT("DeadZoneCollection"),LOCTEXT("DeadZoneCollection_Name", "Controller DeadZone"));
//
// 		Screen->AddSetting(DeadZone);
//
// 		//----------------------------------------------------------------------------------
// 		{
// 			const auto Setting = UGameSettingValueScalarDynamic::CreateSettings(
// 				TEXT("MoveStickDeadZone"),
// 				LOCTEXT("MoveStickDeadZone_Name", "Left Stick DeadZone"),
// 				LOCTEXT("MoveStickDeadZone_Description",
// 				        "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the character continuing to move even after removing your finger from the stick."),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadMoveStickDeadZone),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadMoveStickDeadZone),
// 				GetDefault<ULyraSettingsShared>()->GetGamepadMoveStickDeadZone(),
// 				UGameSettingValueScalarDynamic::ZeroToOnePercent,
// 				FWhenPlayingAsPrimaryPlayer::Get());
//
// 			Setting->SetMinimumLimit(GamepadSensitivityMinimumLimit);
// 			Setting->SetMaximumLimit(GamepadSensitivityMaximumLimit);
//
// 			DeadZone->AddSetting(Setting);
// 		}
// 		//----------------------------------------------------------------------------------
// 		{
// 			const auto Setting = UGameSettingValueScalarDynamic::CreateSettings(
// 				TEXT("LookStickDeadZone"),
// 				LOCTEXT("LookStickDeadZone_Name", "Right Stick DeadZone"),
// 				LOCTEXT("LookStickDeadZone_Description",
// 				        "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the camera continuing to move even after removing your finger from the stick."),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadLookStickDeadZone),
// 				GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadLookStickDeadZone),
// 				GetDefault<ULyraSettingsShared>()->GetGamepadLookStickDeadZone(),
// 				UGameSettingValueScalarDynamic::ZeroToOnePercent,
// 				FWhenPlayingAsPrimaryPlayer::Get());
//
// 			Setting->SetMinimumLimit(GamepadSensitivityMinimumLimit);
// 			Setting->SetMaximumLimit(GamepadSensitivityMaximumLimit);
//
// 			DeadZone->AddSetting(Setting);
// 		}
// 		//----------------------------------------------------------------------------------
// 	}
//
// 	return Screen;
// }

#undef LOCTEXT_NAMESPACE
