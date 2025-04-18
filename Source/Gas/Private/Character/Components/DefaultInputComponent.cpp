// Copyright Epic Games, Inc. All Rights Reserved.


#include "Character/Components/DefaultInputComponent.h"
#include "EnhancedInputSubsystems.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(DefaultInputComponent)

class ULyraInputConfig_DA;

void UDefaultInputComponent::AddInputMappings(const ULyraInputConfig_DA* InputConfig,
                                           const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UDefaultInputComponent::RemoveInputMappings(const ULyraInputConfig_DA* InputConfig,
                                              const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UDefaultInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (const uint32 Handle : BindHandles) { RemoveBindingByHandle(Handle); }
	BindHandles.Reset();
}