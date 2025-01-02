// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/LyraTouchRegionWidget.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraTouchRegionWidget)

struct FGeometry;
struct FPointerEvent;

FReply ULyraTouchRegionWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	bShouldSimulateInput = true;
	return Super::NativeOnTouchStarted(InGeometry, InGestureEvent);
}

FReply ULyraTouchRegionWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	// Input our associatied key as long as the player is touching within our bounds
	//InputKeyValue(FVector::OneVector);
	bShouldSimulateInput = true;
	// Continuously trigger the input if we should
	return Super::NativeOnTouchMoved(InGeometry, InGestureEvent);
}

FReply ULyraTouchRegionWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	bShouldSimulateInput = false;
	return Super::NativeOnTouchEnded(InGeometry, InGestureEvent);
}

void ULyraTouchRegionWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (bShouldSimulateInput) { InputKeyValue(FVector::OneVector); }
}
