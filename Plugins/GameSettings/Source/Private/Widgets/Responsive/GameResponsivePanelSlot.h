// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/PanelSlot.h"
#include "SGameResponsivePanel.h"

#include "GameResponsivePanelSlot.generated.h"

class UObject;

UCLASS()
class UGameResponsivePanelSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()
	void BuildSlot(const TSharedRef<SGameResponsivePanel>& GameResponsivePanel);

	// UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// End of UPanelSlot interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	SGameResponsivePanel::FSlot* Slot;
};
