// Fill out your copyright notice in the Description page of Project Settings.


#include "Policies/GUIPolicy.h"

#include "Tags/BaseGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GUIPolicy)

void UGUIPolicy::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterLayer(UITags::LAYER_GAME, GameLayer_Stack.Get());
	RegisterLayer(UITags::LAYER_GAMEMENU, GameMenu_Stack.Get());
	RegisterLayer(UITags::LAYER_MENU, Menu_Stack.Get());
	RegisterLayer(UITags::LAYER_MODAL, Modal_Stack.Get());
}
