// Fill out your copyright notice in the Description page of Project Settings.


#include "Experience/AsyncAction_ExperienceReady.h"

#include "Experience/ExperienceManagerComponent.h"
#include "GameFramework/GameStateBase.h"

UAsyncAction_ExperienceReady* UAsyncAction_ExperienceReady::WaitForExperienceReady(UObject* WorldContextObject)
{
	const auto World = GEngine->GetWorldFromContextObject(
		WorldContextObject,
		EGetWorldErrorMode::LogAndReturnNull);

	if (!World) { return nullptr; }

	const auto Action = NewObject<UAsyncAction_ExperienceReady>();
	Action->WorldPtr = World;
	Action->RegisterWithGameInstance(World);
	return Action;
}

void UAsyncAction_ExperienceReady::Activate()
{
	Super::Activate();
	const auto World = WorldPtr.Get();
	if (!World)
	{
		// No world so we'll never finish naturally
		SetReadyToDestroy();
		return;
	}

	if (const AGameStateBase* GameState = World->GetGameState())
	{
		Step2_ListenToExperienceLoading(GameState);
		return;
	}
	World->GameStateSetEvent.AddUObject(this, &ThisClass::Step1_HandleGameStateSet);
}

void UAsyncAction_ExperienceReady::Step1_HandleGameStateSet(AGameStateBase* GameState)
{
	if (UWorld* World = WorldPtr.Get()) { World->GameStateSetEvent.RemoveAll(this); }

	Step2_ListenToExperienceLoading(GameState);
}

void UAsyncAction_ExperienceReady::Step2_ListenToExperienceLoading(const AGameStateBase* GameState)
{
	check(GameState);

	const auto ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceComponent);

	if (!ExperienceComponent->IsExperienceLoaded())
	{
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(
			FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::Step3_HandleExperienceLoaded));
		return;
	}

	const auto World = GameState->GetWorld();
	check(World);

	// The experience happened to be already loaded, but still delay a frame to
	// make sure people don't write stuff that relies on this always being true
	//@TODO: Consider not delaying for dynamically spawned stuff / any time after the loading screen has dropped?
	//@TODO: Maybe just inject a random 0-1s delay in the experience load itself?
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::Step4_BroadcastReady));
}

void UAsyncAction_ExperienceReady::Step3_HandleExperienceLoaded(const UExperienceDefinition_DA* CurrentExperience)
{
	Step4_BroadcastReady();
}

void UAsyncAction_ExperienceReady::Step4_BroadcastReady()
{
	OnExperienceReadyAsyncDelegate.Broadcast();
	SetReadyToDestroy();
}
