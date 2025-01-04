// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/GameFeatureAction_AddInputContextMapping.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "InputMappingContext.h"
// #include "Character/LyraHeroComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "AssetManager/BaseAssetManager.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "Character/Components/HeroComponent.h"

#include "Log/Log.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddInputContextMapping)

#define LOCTEXT_NAMESPACE "GameFeatures"


void UGameFeatureAction_AddInputContextMapping::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	RegisterInputMappingContexts();
}

void UGameFeatureAction_AddInputContextMapping::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveData.ControllersAddedTo.IsEmpty()))
		Reset(ActiveData);
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputContextMapping::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData)) Reset(*ActiveData);
}

void UGameFeatureAction_AddInputContextMapping::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();

	UnregisterInputMappingContexts();
}

void UGameFeatureAction_AddInputContextMapping::RegisterInputMappingContexts()
{
	RegisterInputContextMappingsForGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(
		this, &UGameFeatureAction_AddInputContextMapping::RegisterInputContextMappingsForGameInstance);

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator();
	     WorldContextIterator; ++WorldContextIterator) { RegisterInputContextMappingsForGameInstance(WorldContextIterator->OwningGameInstance); }
}

void UGameFeatureAction_AddInputContextMapping::RegisterInputContextMappingsForGameInstance(UGameInstance* GameInstance)
{
	if (!GameInstance || GameInstance->OnLocalPlayerAddedEvent.IsBoundToObject(this)) return;

	GameInstance->OnLocalPlayerAddedEvent.AddUObject(
		this, &ThisClass::RegisterInputMappingContextsForLocalPlayer);

	GameInstance->OnLocalPlayerRemovedEvent.AddUObject(
		this, &ThisClass::UnregisterInputMappingContextsForLocalPlayer);

	for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator();
	     LocalPlayerIterator; ++LocalPlayerIterator) { RegisterInputMappingContextsForLocalPlayer(*LocalPlayerIterator); }
}

void UGameFeatureAction_AddInputContextMapping::RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!(ensure(LocalPlayer))) return;

	const auto EiSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!EiSubsystem) return;

	UEnhancedInputUserSettings* Settings = EiSubsystem->GetUserSettings();
	if (!Settings) return;

	auto& AssetManager = UBaseAssetManager::Get();
	if (!AssetManager.IsInitialized()) return;

	for (const FInputMappingContextAndPriority& Entry : InputMappings)
	{
		// Skip entries that don't want to be registered
		if (!Entry.bRegisterWithSettings) continue;

		// Register this IMC with the settings!
		if (const UInputMappingContext* Imc = AssetManager.GetAsset(Entry.InputMapping)) Settings->RegisterInputMappingContext(Imc);
	}
}

void UGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContexts()
{
	FWorldDelegates::OnStartGameInstance.Remove(RegisterInputContextMappingsForGameInstanceHandle);
	RegisterInputContextMappingsForGameInstanceHandle.Reset();

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator();
	     WorldContextIterator; ++WorldContextIterator) { UnregisterInputContextMappingsForGameInstance(WorldContextIterator->OwningGameInstance); }
}

void UGameFeatureAction_AddInputContextMapping::UnregisterInputContextMappingsForGameInstance(
	UGameInstance* GameInstance)
{
	if (!GameInstance) return;
	GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
	GameInstance->OnLocalPlayerRemovedEvent.RemoveAll(this);

	for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator();
	     LocalPlayerIterator; ++LocalPlayerIterator) { UnregisterInputMappingContextsForLocalPlayer(*LocalPlayerIterator); }
}

void UGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	if (!(ensure(LocalPlayer))) return;

	const auto EiSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!EiSubsystem) return;

	UEnhancedInputUserSettings* Settings = EiSubsystem->GetUserSettings();
	if (!Settings) return;

	for (const auto& [InputMapping, Priority, bRegisterWithSettings] : InputMappings)
	{
		// Skip entries that don't want to be registered
		if (!bRegisterWithSettings) continue;

		// Register this IMC with the settings!
		if (const auto Imc = InputMapping.Get()) Settings->UnregisterInputMappingContext(Imc);
	}
}


#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputContextMapping::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);

	int32 Index = 0;

	for (const FInputMappingContextAndPriority& Entry : InputMappings)
	{
		if (Entry.InputMapping.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullInputMapping", "Null InputMapping at index {0}."), Index));
		}
		++Index;
	}

	return Result;
}
#endif

void UGameFeatureAction_AddInputContextMapping::AddToWorld(const FWorldContext& WorldContext,
                                                           const FGameFeatureStateChangeContext& ChangeContext)
{
	const UWorld* World = WorldContext.World();

	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	if (!GameInstance || !World || !World->IsGameWorld()) return;

	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	const auto ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
	if (!ComponentManager) return;

	const UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
		UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
			this, &ThisClass::HandleControllerExtension, ChangeContext);

	const TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
		ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), AddAbilitiesDelegate);

	ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
}

void UGameFeatureAction_AddInputContextMapping::Reset(FPerContextData& ActiveData)
{
	ActiveData.ExtensionRequestHandles.Empty();

	while (!ActiveData.ControllersAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APlayerController> ControllerPtr = ActiveData.ControllersAddedTo.Top();
		if (ControllerPtr.IsValid()) RemoveInputMapping(ControllerPtr.Get(), ActiveData);
		else ActiveData.ControllersAddedTo.Pop();
	}
}

void UGameFeatureAction_AddInputContextMapping::HandleControllerExtension(
	AActor* Actor, const FName EventName, const FGameFeatureStateChangeContext ChangeContext)
{
	APlayerController* AsController = CastChecked<APlayerController>(Actor);
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// TODO Why does this code mix and match controllers and local players? ControllersAddedTo is never modified
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveInputMapping(AsController, ActiveData);
		return;
	}

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded ||
		EventName == UHeroComponent::NAME_BIND_INPUTS_NOW)
		AddInputMappingForPlayer(AsController, ActiveData);
}

void UGameFeatureAction_AddInputContextMapping::AddInputMappingForPlayer(const APlayerController* PlayerController, FPerContextData& ActiveData)
{
	if (!PlayerController)
	{
		UE_LOG(LogGameFeatures, Error, TEXT("PlayerController is null"));
		return;
	}
	const auto LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogGameFeatures, Error, TEXT("LocalPlayer is null"));
		return;
	}
	const auto InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSystem)
	{
		UE_LOG(LogGameFeatures, Error,
		       TEXT(
			       "Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file."
		       ));
		return;
	}

	for (const auto [InputMapping, Priority, bRegisterWithSettings] : InputMappings)
	{
		const auto Imc = InputMapping.Get();
		if (!Imc) continue;
		InputSystem->AddMappingContext(Imc, Priority);
	}
}

void UGameFeatureAction_AddInputContextMapping::RemoveInputMapping(APlayerController* PlayerController, FPerContextData& ActiveData)
{
	if (!PlayerController)
	{
		UE_LOG(LogGameFeatures, Error, TEXT("PlayerController is null"));
		return;
	}
	const auto LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogGameFeatures, Error, TEXT("LocalPlayer is null"));
		return;
	}
	const auto InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSystem)
	{
		UE_LOG(LogGameFeatures, Error,
		       TEXT(
			       "Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file."
		       ));
		return;
	}

	for (const auto IM : InputMappings)
	{
		const auto Imc = IM.InputMapping.Get();
		if (!Imc) continue;
		InputSystem->RemoveMappingContext(Imc);
	}

	ActiveData.ControllersAddedTo.Remove(PlayerController);
}

#undef LOCTEXT_NAMESPACE