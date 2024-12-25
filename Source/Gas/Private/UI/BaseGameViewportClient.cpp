// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BaseGameViewportClient.h"

#include "CommonUISettings.h"
#include "ICommonUIModule.h"
#include "Tags/BaseGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGameViewportClient)

void UBaseGameViewportClient::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance,
                                   bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);

	// We have software cursors set up in our project settings for console/mobile use, but on desktop we're fine with
	// the standard hardware cursors
	const bool UseHardwareCursor = ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(
		PlatformTags::TRAIT_INPUT_HARDWARECURSOR);
	SetUseSoftwareCursorWidgets(!UseHardwareCursor);
}
