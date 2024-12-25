// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/BaseActivatableWidget.h"
#include "BaseHUDLayout.generated.h"

/**
 * 
*
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Meta = (DisplayName = "Base HUD Layout", Category = "Base|HUD"))
class GAS_API UBaseHUDLayout : public UBaseActivatableWidget
{
	GENERATED_BODY()

public:
	UBaseHUDLayout(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer) {}

	virtual void NativeOnInitialized() override;

protected:
	void HandleEscapeAction();

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};
