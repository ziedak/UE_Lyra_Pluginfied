// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PrimaryGameLayout.h"
#include "GUIPolicy.generated.h"

/**
* That's where we register the different layer widgets that you can send content to.

* The layers are,
* Game - Things like the HUD.
* GameMenu - "Menus" specifically related to gameplay, like maybe an in game inventory UI.
* Menu - Things like the settings screen.
* Modal - Confirmation dialogs, error dialogs.
* 
* You will push/pop or push/deactivate to different layers. If something new is put onto the GameMenu layer,
* while another thing is on it, it will stop displaying the old UI, and show the new one until the new one is
* deactivated (which pops it).
* 
* If you put something on an entirely different layer, like the Menu, all layers below it will still be visible,
* because each of these layers is a separate stack of widgets.
* 
* The Layer widgets can be any widget that subclasses from UCommonActivatableWidgetContainerBase,
* so if you wanted you could make the modal stack a UCommonActivatableWidgetQueue,
* and suddenly any time you pushed modal content onto the modal layer, it would operate like a queue,
* waiting for the modal in front to be dealt with before the modal next in line can activate.
 */

UCLASS()
class GAS_API UGUIPolicy : public UPrimaryGameLayout
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta=(DisplayName="GameLayer_Stack Class", BindWidget))
	TSoftObjectPtr<UCommonActivatableWidgetStack> GameLayer_Stack;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta=(DisplayName="GameLayer_Stack Class", BindWidget))
	TSoftObjectPtr<UCommonActivatableWidgetStack> GameMenu_Stack;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta=(DisplayName="GameLayer_Stack Class", BindWidget))
	TSoftObjectPtr<UCommonActivatableWidgetStack> Menu_Stack;

	UPROPERTY(EditDefaultsOnly, Category = "UI", meta=(DisplayName="GameLayer_Stack Class", BindWidget))
	TSoftObjectPtr<UCommonActivatableWidgetStack> Modal_Stack;
};
