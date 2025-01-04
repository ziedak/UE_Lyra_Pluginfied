// Copyright Epic Games, Inc. All Rights Reserved.

#include "Subsystem/LyraLoadingScreenSubsystem.h"
#include "Blueprint/UserWidget.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLoadingScreenSubsystem)

class UUserWidget;

ULyraLoadingScreenSubsystem::ULyraLoadingScreenSubsystem() {}

void ULyraLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (LoadingScreenWidgetClass == NewWidgetClass) { return; }

	LoadingScreenWidgetClass = NewWidgetClass;
	OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
}

TSubclassOf<UUserWidget> ULyraLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}