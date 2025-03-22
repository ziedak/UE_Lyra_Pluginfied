// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actions/AsyncAction_PushContentToLayerForPlayer.h"

#include "Engine/Engine.h"
#include "PrimaryGameLayout.h"
#include "UObject/Stack.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_PushContentToLayerForPlayer)

UAsyncAction_PushContentToLayerForPlayer::UAsyncAction_PushContentToLayerForPlayer(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

UAsyncAction_PushContentToLayerForPlayer* UAsyncAction_PushContentToLayerForPlayer::PushContentToLayerForPlayer(
	APlayerController* InOwningPlayer, const TSoftClassPtr<UCommonActivatableWidget> InWidgetClass, const FGameplayTag InLayerName,
	const bool bSuspendInputUntilComplete)
{
	if (InWidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(
			TEXT("PushContentToLayerForPlayer was passed a null WidgetClass"), ELogVerbosity::Error);
		return nullptr;
	}
	const UWorld* World = GEngine->GetWorldFromContextObject(InOwningPlayer, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;
	UAsyncAction_PushContentToLayerForPlayer* Action = NewObject<UAsyncAction_PushContentToLayerForPlayer>();
	Action->WidgetClass = InWidgetClass;
	Action->OwningPlayerPtr = InOwningPlayer;
	Action->LayerName = InLayerName;
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UAsyncAction_PushContentToLayerForPlayer::Cancel()
{
	Super::Cancel();

	if (!StreamingHandle.IsValid()) return;
	StreamingHandle->CancelHandle();
	StreamingHandle.Reset();
}

void UAsyncAction_PushContentToLayerForPlayer::Activate()
{
	UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayout(OwningPlayerPtr.Get());
	if (!RootLayout)
	{
		SetReadyToDestroy();
		return;
	}
	TWeakObjectPtr<UAsyncAction_PushContentToLayerForPlayer> WeakThis = this;
	StreamingHandle = RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(
		LayerName, bSuspendInputUntilComplete, WidgetClass,
		[this, WeakThis](const EAsyncWidgetLayerState State, UCommonActivatableWidget* Widget){
			if (WeakThis.IsValid())
			{
				switch (State)
				{
				case EAsyncWidgetLayerState::Initialize: BeforePush.Broadcast(Widget);
					break;
				case EAsyncWidgetLayerState::AfterPush: AfterPush.Broadcast(Widget);
					SetReadyToDestroy();
					break;
				case EAsyncWidgetLayerState::Canceled: SetReadyToDestroy();
					break;
				}
			}
			SetReadyToDestroy();
		});
}