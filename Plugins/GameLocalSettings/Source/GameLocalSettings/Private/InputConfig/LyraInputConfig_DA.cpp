// Copyright Epic Games, Inc. All Rights Reserved.

#include "InputConfig/LyraInputConfig_DA.h"

#include "GameplayTagContainer.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInputConfig_DA)


const UInputAction* ULyraInputConfig_DA::FindNativeInputActionForTag(const FGameplayTag& InInputTag,
                                                                     const bool bLogNotFound) const
{
	for (const auto& [InputAction, InputTag] : NativeInputActions)
	{
		if (InputAction && (InputTag == InInputTag)) { return InputAction; }
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."),
		       *InInputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* ULyraInputConfig_DA::FindAbilityInputActionForTag(const FGameplayTag& InInputTag,
                                                                      const bool bLogNotFound) const
{
	//  Iterate through the AbilityInputActions array and return the InputAction that matches the InputTag.
	for (const auto& [InputAction, InputTag] : AbilityInputActions)
	{
		if (InputAction && InputTag == InInputTag) { return InputAction; }
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."),
		       *InInputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
