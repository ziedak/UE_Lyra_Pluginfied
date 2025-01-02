// Copyright Epic Games, Inc. All Rights Reserved.

#include "IndicatorSystem/LyraIndicatorManagerComponent.h"

#include "IndicatorSystem/IndicatorDescriptor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraIndicatorManagerComponent)

ULyraIndicatorManagerComponent::ULyraIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRegister = true;
	bAutoActivate = true;
}

/*static*/
ULyraIndicatorManagerComponent* ULyraIndicatorManagerComponent::GetComponent(const AController* Controller)
{
	if (Controller) { return Controller->FindComponentByClass<ULyraIndicatorManagerComponent>(); }

	return nullptr;
}

void ULyraIndicatorManagerComponent::AddIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	IndicatorDescriptor->SetIndicatorManagerComponent(this);
	OnIndicatorAdded.Broadcast(IndicatorDescriptor);
	Indicators.Add(IndicatorDescriptor);
}

void ULyraIndicatorManagerComponent::RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	if (!IndicatorDescriptor) { return; }

	ensure(IndicatorDescriptor->GetIndicatorManagerComponent() == this);
	OnIndicatorRemoved.Broadcast(IndicatorDescriptor);
	Indicators.Remove(IndicatorDescriptor);
}
