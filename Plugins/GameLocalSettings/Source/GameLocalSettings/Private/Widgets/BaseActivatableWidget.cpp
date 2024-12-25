// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/BaseActivatableWidget.h"
#include "Editor/WidgetCompilerLog.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseActivatableWidget)

#define LOCTEXT_NAMESPACE "Lyra"

TOptional<FUIInputConfig> UBaseActivatableWidget::GetDesiredInputConfig() const
{
	switch (InputConfig)
	{
	case EWidgetInputMode::GameAndMenu:
		return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
	case EWidgetInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
	case EWidgetInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
	case EWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}
#if WITH_EDITOR

void UBaseActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree,
                                                        class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);
	const auto IsFunctionImplement = GetClass()->IsFunctionImplementedInScript(
		GET_FUNCTION_NAME_CHECKED(UBaseActivatableWidget, BP_GetDesiredFocusTarget));
	if (IsFunctionImplement) { return; }

	if (GetParentNativeClass(GetClass()) == StaticClass())
	{
		CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning",
		                           "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		return;
	}

	//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
	CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note",
	                        "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
}

#endif

#undef LOCTEXT_NAMESPACE
