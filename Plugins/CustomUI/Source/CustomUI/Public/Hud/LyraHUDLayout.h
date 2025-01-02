// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/LyraActivatableWidget.h"

#include "LyraHUDLayout.generated.h"

class UCommonActivatableWidget;
class UObject;


/**
 * ULyraHUDLayout
 *
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "Lyra HUD Layout", Category = "Lyra|HUD"))
class CUSTOMUI_API ULyraHUDLayout : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:
	ULyraHUDLayout(const FObjectInitializer& ObjectInitializer);

	virtual void NativeOnInitialized() override;

protected:
	virtual void HandleEscapeAction() const;

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};
