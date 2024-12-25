// Copyright Epic Games, Inc. All Rights Reserved.


#include "InputConfig/LyraInputComponent.h"
#include "EnhancedInputSubsystems.h"


// #include "Player/LyraLocalPlayer.h"
// #include "Settings/LyraSettingsLocal.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInputComponent)

class ULyraInputConfig_DA;

void ULyraInputComponent::AddInputMappings(const ULyraInputConfig_DA* InputConfig,
                                           const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void ULyraInputComponent::RemoveInputMappings(const ULyraInputConfig_DA* InputConfig,
                                              const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void ULyraInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (const uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}
