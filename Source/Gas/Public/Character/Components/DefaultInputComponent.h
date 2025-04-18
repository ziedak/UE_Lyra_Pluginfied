// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EnhancedInputComponent.h"
 #include "InputConfig/LyraInputConfig_DA.h"

#include "DefaultInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UObject;
//class ULyraInputConfig_DA;
struct FGameplayTag;
/**
 * ULyraInputComponent
 *
 *	Component used to manage input mappings and bindings using an input config data asset.
 */
UCLASS(Config = Input)
class GAS_API UDefaultInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UDefaultInputComponent(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer) {}

	void AddInputMappings(const ULyraInputConfig_DA* InputConfig,
	                      const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const ULyraInputConfig_DA* InputConfig,
	                         const UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	// This function binds an input action to a specific function in a user-defined class.
	// The FuncType should match the expected signature for the input action, which is usually a member function of UserClass.
	// void (UserClass::*Func)(const FInputActionValue&);
	// If the function is const, it should be:
	// void (UserClass::*Func)(const FInputActionValue&) const;
	template <class UserClass, typename FuncType>
	void BindNativeAction(const ULyraInputConfig_DA* InputConfig, const FGameplayTag& InputTag,
	                      ETriggerEvent TriggerEvent,
	                      UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActionList(const ULyraInputConfig_DA* InputConfig, UserClass* Object, PressedFuncType PressedFunc,
	                           ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UDefaultInputComponent::BindNativeAction(const ULyraInputConfig_DA* InputConfig, const FGameplayTag& InputTag,
                                           ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func,
                                           const bool bLogIfNotFound)
{
	check(InputConfig);
	if (const auto InputAction = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(InputAction, TriggerEvent, Object, Func);
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UDefaultInputComponent::BindAbilityActionList(const ULyraInputConfig_DA* InputConfig, UserClass* Object,
                                                PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc,
                                                TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const auto& Action : InputConfig->AbilityInputActions)
	{
		if (!Action.InputAction || !Action.InputTag.IsValid()) { continue; }

		if (PressedFunc)
		{
			BindHandles.Add(
				BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).
				GetHandle());
		}

		if (ReleasedFunc)
		{
			BindHandles.Add(
				BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).
				GetHandle());
		}
	}
}
