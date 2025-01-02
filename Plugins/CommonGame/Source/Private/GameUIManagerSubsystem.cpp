// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameUIManagerSubsystem.h"

#include "CommonGameSettings.h"

#include "Engine/GameInstance.h"
#include "GameUIPolicy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIManagerSubsystem)

class FSubsystemCollectionBase;
class UClass;


void UGameUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UCommonGameSettings* Settings = GetDefault<UCommonGameSettings>();
	// TSubclassOf<UGameUIPolicy> DefaultUIPolicyClass = Settings->DefaultUIPolicyClass.TryLoadClass<UGameUIPolicy>();
	TSubclassOf<UGameUIPolicy> DefaultUIPolicyClass2 = Settings->DefaultUIPolicyClass.LoadSynchronous();

	if (CurrentPolicy) return;

	// const auto Policy1 = NewObject<UGameUIPolicy>(this, DefaultUIPolicyClass);
	// SwitchToPolicy(Policy1);
	const auto Policy2 = NewObject<UGameUIPolicy>(this, DefaultUIPolicyClass2);
	SwitchToPolicy(Policy2);
}

// void UGameUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
// {
// 	Super::Initialize(Collection);
// 	
// 	if (CurrentPolicy || DefaultUIPolicyClass.IsNull()) { return; }
// 	
// 	const TSubclassOf<UGameUIPolicy> PolicyClass = DefaultUIPolicyClass.LoadSynchronous();
// 	const auto Policy = NewObject<UGameUIPolicy>(this, PolicyClass);
// 	SwitchToPolicy(Policy);
// }

void UGameUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	SwitchToPolicy(nullptr);
}

bool UGameUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// Only create an instance if there is no override implementation defined elsewhere
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UGameUIManagerSubsystem::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer) { if (ensure(LocalPlayer) && CurrentPolicy) CurrentPolicy->NotifyPlayerAdded(LocalPlayer); }

void UGameUIManagerSubsystem::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer) { if (LocalPlayer && CurrentPolicy) CurrentPolicy->NotifyPlayerRemoved(LocalPlayer); }

void UGameUIManagerSubsystem::NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer) { if (LocalPlayer && CurrentPolicy) CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer); }

void UGameUIManagerSubsystem::SwitchToPolicy(UGameUIPolicy* InPolicy) { if (CurrentPolicy != InPolicy) CurrentPolicy = InPolicy; }