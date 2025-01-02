// Copyright Epic Games, Inc. All Rights Reserved.

#include "Common/LyraBoundActionButton.h"

#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraBoundActionButton)

class UCommonButtonStyle;

void ULyraBoundActionButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
		HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
	}
}

void ULyraBoundActionButton::HandleInputMethodChanged(const ECommonInputType NewInputMethod)
{
	TSubclassOf<UCommonButtonStyle> NewStyle;

	if (NewInputMethod == ECommonInputType::Gamepad) { NewStyle = GamepadStyle; }
	else if (NewInputMethod == ECommonInputType::Touch) { NewStyle = TouchStyle; }
	else { NewStyle = KeyboardStyle; }

	if (NewStyle) { SetStyle(NewStyle); }
}
