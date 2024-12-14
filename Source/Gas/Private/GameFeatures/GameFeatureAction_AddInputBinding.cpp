// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/GameFeatureAction_AddInputBinding.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
// #include "Character/LyraHeroComponent.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "InputConfig/LyraInputConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "Character/Components/HeroComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddInputBinding)

#define LOCTEXT_NAMESPACE "GameFeatures"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddInputBinding

void UGameFeatureAction_AddInputBinding::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
	    !ensure(ActiveData.PawnsAddedTo.IsEmpty()))
	{
		Reset(ActiveData);
	}
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputBinding::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FPerContextData* ActiveData = ContextData.Find(Context);

	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputBinding::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);

	int32 Index = 0;

	for (const TSoftObjectPtr<const ULyraInputConfig>& Entry : InputConfigs)
	{
		if (Entry.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullInputConfig", "Null InputConfig at index {0}."), Index));
		}
		++Index;
	}

	return Result;
}
#endif

void UGameFeatureAction_AddInputBinding::AddToWorld(const FWorldContext& WorldContext,
                                                    const FGameFeatureStateChangeContext& ChangeContext)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if (!GameInstance || !World || !World->IsGameWorld())
	{
		return;
	}

	UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<
		UGameFrameworkComponentManager>(GameInstance);
	if (!ComponentManager)
	{
		return;
	}

	const UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
		UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
			this, &ThisClass::HandlePawnExtension, ChangeContext);

	const TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
		ComponentManager->AddExtensionHandler(APawn::StaticClass(), AddAbilitiesDelegate);

	ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
}

void UGameFeatureAction_AddInputBinding::Reset(FPerContextData& ActiveData)
{
	ActiveData.ExtensionRequestHandles.Empty();

	while (!ActiveData.PawnsAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APawn> PawnPtr = ActiveData.PawnsAddedTo.Top();
		if (PawnPtr.IsValid())
		{
			RemoveInputMapping(PawnPtr.Get(), ActiveData);
		}
		else
		{
			ActiveData.PawnsAddedTo.Pop();
		}
	}
}

void UGameFeatureAction_AddInputBinding::HandlePawnExtension(AActor* Actor, FName EventName,
                                                             FGameFeatureStateChangeContext ChangeContext)
{
	APawn* AsPawn = CastChecked<APawn>(Actor);
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName ==
		    UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveInputMapping(AsPawn, ActiveData);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName ==
		         UHeroComponent::NAME_BIND_INPUTS_NOW))
	{
		AddInputMappingForPlayer(AsPawn, ActiveData);
	}
}

void UGameFeatureAction_AddInputBinding::AddInputMappingForPlayer(APawn* Pawn, FPerContextData& ActiveData)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;

	if (!LocalPlayer)
	{
		return;
	}

	const UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSystem)
	{
		UE_LOG(LogGameFeatures, Error,
		       TEXT(
			       "Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. "
			       "Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file."
		       ));
		return;
	}


	UHeroComponent* HeroComponent = Pawn->FindComponentByClass<UHeroComponent>();
	if (HeroComponent && HeroComponent->IsReadyToBindInputs())
	{
		for (const TSoftObjectPtr<const ULyraInputConfig>& Entry : InputConfigs)
		{
			if (const ULyraInputConfig* BindSet = Entry.Get())
			{
				HeroComponent->AddAdditionalInputConfig(BindSet);
			}
		}
	}

	ActiveData.PawnsAddedTo.AddUnique(Pawn);
}

void UGameFeatureAction_AddInputBinding::RemoveInputMapping(APawn* Pawn, FPerContextData& ActiveData)
{
	const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());

	if (const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>())
		{
			if (const UHeroComponent* HeroComponent = Pawn->FindComponentByClass<UHeroComponent>())
			{
				for (const TSoftObjectPtr<const ULyraInputConfig>& Entry : InputConfigs)
				{
					if (const ULyraInputConfig* InputConfig = Entry.Get())
					{
						HeroComponent->RemoveAdditionalInputConfig(InputConfig);
					}
				}
			}
		}
	}

	ActiveData.PawnsAddedTo.Remove(Pawn);
}

#undef LOCTEXT_NAMESPACE
