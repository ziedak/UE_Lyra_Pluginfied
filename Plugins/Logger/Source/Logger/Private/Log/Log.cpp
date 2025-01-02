// Fill out your copyright notice in the Description page of Project Settings.


#include "Log/Log.h"

DEFINE_LOG_CATEGORY (LogGAS);
DEFINE_LOG_CATEGORY (LogCORE);
DEFINE_LOG_CATEGORY (LogExperience);


FString GetClientServerContext(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	if (const AActor* Actor = Cast<AActor>(ContextObject)) { Role = Actor->GetLocalRole(); }
	else if (const UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None) { return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client"); }
#if WITH_EDITOR
	if (GIsEditor)
	{
		extern ENGINE_API FString GPlayInEditorContextString;
		return GPlayInEditorContextString;
	}
#endif

	return TEXT("[]");
}

void PrintCallStack() { FDebug::DumpStackTraceToLog(ELogVerbosity::Error); }

void ConsoleLog(const FString& Message)
{
#if WITH_EDITOR
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message); }
#endif
}

void DrawDebugAtPosition(const UWorld* World, const FVector& Location, const FString& Text, const FColor Color,
                         const float Duration)
{
#if WITH_EDITOR
	if (World) { DrawDebugString(World, Location, Text, nullptr, Color, Duration); }
#endif
}
