// Copyright Epic Games, Inc. All Rights Reserved.

#include "MessageRuntime/GameplayMessageSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayMessageSubsystem)

DEFINE_LOG_CATEGORY(LogGameplayMessageSubsystem);

namespace UE::GameplayMessageSubsystem
{
	static int32 ShouldLogMessages = 0;
	static FAutoConsoleVariableRef CVarShouldLogMessages(TEXT("GameplayMessageSubsystem.LogMessages"),
	                                                     ShouldLogMessages,
	                                                     TEXT(
		                                                     "Should messages broadcast through the gameplay message subsystem be logged?"));
}

//////////////////////////////////////////////////////////////////////
// FGameplayMessageListenerHandle

void FGameplayMessageListenerHandle::Unregister()
{
	if (UGameplayMessageSubsystem* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->UnregisterListener(*this);
		Subsystem.Reset();
		Channel = FGameplayTag();
		ID = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// UGameplayMessageSubsystem

UGameplayMessageSubsystem& UGameplayMessageSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<
		UGameplayMessageSubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}

bool UGameplayMessageSubsystem::HasInstance(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UGameplayMessageSubsystem* Router = World != nullptr
		                                    ? UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(
			                                    World->GetGameInstance())
		                                    : nullptr;
	return Router != nullptr;
}

void UGameplayMessageSubsystem::Deinitialize()
{
	ListenerMap.Reset();

	Super::Deinitialize();
}

void UGameplayMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType,
                                                         const void* MessageBytes)
{
	// Log the message if enabled
	if (UE::GameplayMessageSubsystem::ShouldLogMessages != 0)
	{
		const FString* PContextString = nullptr;
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			PContextString = &GPlayInEditorContextString;
		}
#endif

		FString HumanReadableMessage;
		StructType->ExportText(/*out*/ HumanReadableMessage, MessageBytes, /*Defaults=*/ nullptr, /*OwnerObject=*/
		                               nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
		UE_LOG(LogGameplayMessageSubsystem, Log, TEXT("BroadcastMessage(%s, %s, %s)"),
		       PContextString ? **PContextString : *GetPathNameSafe(this), *Channel.ToString(), *HumanReadableMessage);
	}

	// Broadcast the message
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
		{
			// Copy in case there are removals while handling callbacks
			TArray<FGameplayMessageListenerData> ListenerArray(pList->Listeners);

			for (const FGameplayMessageListenerData& Listener : ListenerArray)
			{
				if (!bOnInitialTag && (Listener.MatchType != EGameplayMessageMatch::PartialMatch))
				{
					UE_LOG(LogGameplayMessageSubsystem, Warning,
					       TEXT("Listener on channel %s is not a partial match, skipping"), *Channel.ToString());
					continue;
				}

				if (Listener.bHadValidType && !Listener.ListenerStructType.IsValid())
				{
					UE_LOG(LogGameplayMessageSubsystem, Warning,
					       TEXT("Listener struct type has gone invalid on Channel %s. Removing listener from list"),
					       *Channel.ToString());
					UnregisterListenerInternal(Channel, Listener.HandleID);
					continue;
				}

				// The receiving type must be either a parent of the sending type or completely ambiguous (for internal use)
				if (Listener.bHadValidType && !StructType->IsChildOf(Listener.ListenerStructType.Get()))
				{
					UE_LOG(LogGameplayMessageSubsystem, Error,
					       TEXT(
						       "Struct type mismatch on channel %s (broadcast type %s, listener at %s was expecting type %s)"
					       ),
					       *Channel.ToString(),
					       *StructType->GetPathName(),
					       *Tag.ToString(),
					       *Listener.ListenerStructType->GetPathName());
					continue;
				}

				Listener.ReceivedCallback(Channel, StructType, MessageBytes);
			}
		}
		bOnInitialTag = false;
	}
}

void UGameplayMessageSubsystem::K2_BroadcastMessage(FGameplayTag Channel, const int32& Message)
{
	// This will never be called, the exec version below will be hit instead
	checkNoEntry();
}

DEFINE_FUNCTION(UGameplayMessageSubsystem::execK2_BroadcastMessage)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr)))
	{
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct, MessagePtr);
	}
}

FGameplayMessageListenerHandle UGameplayMessageSubsystem::RegisterListenerInternal(
	const FGameplayTag Channel, TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
	const UScriptStruct* StructType, EGameplayMessageMatch MatchType)
{
	auto& [Listeners, HandleID] = ListenerMap.FindOrAdd(Channel);

	FGameplayMessageListenerData& Entry = Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.bHadValidType = StructType != nullptr;
	Entry.HandleID = ++HandleID;
	Entry.MatchType = MatchType;

	return FGameplayMessageListenerHandle(this, Channel, Entry.HandleID);
}

void UGameplayMessageSubsystem::UnregisterListener(const FGameplayMessageListenerHandle& Handle)
{
	if (!Handle.IsValid())
	{
		UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("Trying to unregister an invalid Handle."));
		return;
	}

	check(Handle.Subsystem == this);
	UnregisterListenerInternal(Handle.Channel, Handle.ID);
}

void UGameplayMessageSubsystem::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	FChannelListenerList* pList = ListenerMap.Find(Channel);
	if (!pList)
	{
		return;
	}

	const int32 MatchIndex = pList->Listeners.IndexOfByPredicate(
		[ID = HandleID](const FGameplayMessageListenerData& Other) { return Other.HandleID == ID; });
	if (MatchIndex != INDEX_NONE)
	{
		pList->Listeners.RemoveAtSwap(MatchIndex);
	}

	if (pList->Listeners.Num() == 0)
	{
		ListenerMap.Remove(Channel);
	}
}
