// Copyright Epic Games, Inc. All Rights Reserved.

#include "Hud/LyraHUD.h"

#include "Async/TaskGraphInterfaces.h"
#include "Components/GameFrameworkComponentManager.h"
#include "UObject/UObjectIterator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraHUD)

class AActor;
class UWorld;

ALyraHUD::ALyraHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) { PrimaryActorTick.bStartWithTickEnabled = false; }

void ALyraHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ALyraHUD::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
		this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void ALyraHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}
