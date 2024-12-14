// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/GameFeatureAction_AddAbilities.h"
#include "Engine/GameInstance.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Engine/World.h"
//#include "Player/BasePlayerState.h" //@TODO: For the fname
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "Ability/BaseAbilitySet.h"
#include "AbilitySet/GrantedHandlesData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddAbilities)

#define LOCTEXT_NAMESPACE "GameFeatures"


void UGameFeatureAction_AddAbilities::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);

	if (!ensureAlways(ActiveData.ActiveExtensions.IsEmpty()) ||
	    !ensureAlways(ActiveData.ComponentRequests.IsEmpty()))
	{
		Reset(ActiveData);
	}
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FPerContextData* ActiveData = ContextData.Find(Context);

	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddAbilities::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(
				LOCTEXT("EntryHasNullActor", "Null ActorClass at index {0} in AbilitiesList"),
				FText::AsNumber(EntryIndex)));
		}

		if (Entry.GrantedAbilities.IsEmpty() && Entry.GrantedAttributes.IsEmpty() && Entry.GrantedAbilitySets.IsEmpty())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("EntryHasNoAddOns",
			                                       "Index {0} in AbilitiesList will do nothing (no granted abilities, attributes, or ability sets)"), FText::AsNumber(
				                               EntryIndex)));
		}

		int32 AbilityIndex = 0;
		for (const FAbilityGrant& Ability : Entry.GrantedAbilities)
		{
			if (Ability.AbilityType.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(
					LOCTEXT("EntryHasNullAbility",
					        "Null AbilityType at index {0} in AbilitiesList[{1}].GrantedAbilities"),
					FText::AsNumber(AbilityIndex), FText::AsNumber(EntryIndex)));
			}
			++AbilityIndex;
		}

		int32 AttributesIndex = 0;
		for (const FAttributeSetGrant& Attributes : Entry.GrantedAttributes)
		{
			if (Attributes.AttributeSetType.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(
					LOCTEXT("EntryHasNullAttributeSet",
					        "Null AttributeSetType at index {0} in AbilitiesList[{1}].GrantedAttributes"),
					FText::AsNumber(AttributesIndex), FText::AsNumber(EntryIndex)));
			}
			++AttributesIndex;
		}

		int32 AttributeSetIndex = 0;
		for (const TSoftObjectPtr<const UBaseAbilitySet>& AttributeSetPtr : Entry.GrantedAbilitySets)
		{
			if (AttributeSetPtr.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(
					LOCTEXT("EntryHasNullAttributeSet",
					        "Null AbilitySet at index {0} in AbilitiesList[{1}].GrantedAbilitySets"),
					FText::AsNumber(AttributeSetIndex), FText::AsNumber(EntryIndex)));
			}
			++AttributeSetIndex;
		}
		++EntryIndex;
	}

	return Result;

	//	return EDataValidationResult::NotValidated;
}
#endif

void UGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext,
                                                 const FGameFeatureStateChangeContext& ChangeContext)
{
	const UWorld* World = WorldContext.World();
	const UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if (!GameInstance || !World || !World->IsGameWorld())
	{
		return;
	}
	UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<
		UGameFrameworkComponentManager>(GameInstance);
	if (!ComponentMan)
	{
		return;
	}

	int32 EntryIndex = 0;
	for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
	{
		if (Entry.ActorClass.IsNull())
		{
			continue;
		}

		const UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
			UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
				this, &UGameFeatureAction_AddAbilities::HandleActorExtension, EntryIndex, ChangeContext);
		TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(
			Entry.ActorClass, AddAbilitiesDelegate);

		ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
		EntryIndex++;
	}
}


void UGameFeatureAction_AddAbilities::Reset(FPerContextData& ActiveData) const
{
	while (!ActiveData.ActiveExtensions.IsEmpty())
	{
		const auto ExtensionIt = ActiveData.ActiveExtensions.CreateIterator();
		RemoveActorAbilities(ExtensionIt->Key, ActiveData);
	}

	ActiveData.ComponentRequests.Empty();
}

void UGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor,
                                                           FName EventName,
                                                           int32 EntryIndex,
                                                           FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (!AbilitiesList.IsValidIndex(EntryIndex) || !ActiveData)
	{
		return;
	}

	const FGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName ==
		    UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveActorAbilities(Actor, *ActiveData);
	}
	//@todo Implement this
	// else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == ABasePlayerState::NAME_BaseAbilityReady))
	// {
	// 	AddActorAbilities(Actor, Entry, *ActiveData);
	// }
}

//  Add abilities to the actor if they don't already have them and are on the server 
void UGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry,
                                                        FPerContextData& ActiveData)
{
	check(Actor);
	if (!Actor->HasAuthority())
	{
		return;
	}

	// early out if Actor already has ability extensions applied
	if (ActiveData.ActiveExtensions.Find(Actor))
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = FindOrAddComponentForActor<UAbilitySystemComponent>(
		Actor, AbilitiesEntry, ActiveData);
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogGameFeatures, Error,
		       TEXT("Failed to find/add an ability component to '%s'. Abilities will not be granted."),
		       *Actor->GetPathName());
		return;
	}

	// Add abilities to the actor 
	FActorExtensions AddedExtensions;
	AddedExtensions.Abilities.Reserve(AbilitiesEntry.GrantedAbilities.Num());
	AddedExtensions.Attributes.Reserve(AbilitiesEntry.GrantedAttributes.Num());
	AddedExtensions.AbilitySetHandles.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());

	for (const auto& [AbilityType] : AbilitiesEntry.GrantedAbilities)
	{
		if (AbilityType.IsNull())
		{
			continue;
		}
		FGameplayAbilitySpec NewAbilitySpec(AbilityType.LoadSynchronous());
		FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);

		AddedExtensions.Abilities.Add(AbilityHandle);
	}

	for (const FAttributeSetGrant& Attributes : AbilitiesEntry.GrantedAttributes)
	{
		if (Attributes.AttributeSetType.IsNull())
		{
			continue;
		}

		TSubclassOf<UAttributeSet> SetType = Attributes.AttributeSetType.LoadSynchronous();
		if (!SetType)
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(), SetType);
		if (!Attributes.InitializationData.IsNull())
		{
			UDataTable* InitData = Attributes.InitializationData.LoadSynchronous();
			if (InitData)
			{
				NewSet->InitFromMetaDataTable(InitData);
			}
		}

		AddedExtensions.Attributes.Add(NewSet);
		AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
	}

	UBaseAbilitySystemComponent* BaseASC = CastChecked<UBaseAbilitySystemComponent>(AbilitySystemComponent);
	for (const TSoftObjectPtr<const UBaseAbilitySet>& SetPtr : AbilitiesEntry.GrantedAbilitySets)
	{
		if (const UBaseAbilitySet* Set = SetPtr.Get())
		{
			Set->GiveToAbilitySystem(BaseASC, &AddedExtensions.AbilitySetHandles.AddDefaulted_GetRef());
		}
	}

	ActiveData.ActiveExtensions.Add(Actor, AddedExtensions);
}

void UGameFeatureAction_AddAbilities::RemoveActorAbilities(const AActor* Actor, FPerContextData& ActiveData) const
{
	FActorExtensions* ActorExtensions = ActiveData.ActiveExtensions.Find(Actor);
	if (!ActorExtensions)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = Actor->FindComponentByClass<UAbilitySystemComponent>())
	{
		for (UAttributeSet* AttribSetInstance : ActorExtensions->Attributes)
		{
			AbilitySystemComponent->RemoveSpawnedAttribute(AttribSetInstance);
		}

		for (const FGameplayAbilitySpecHandle AbilityHandle : ActorExtensions->Abilities)
		{
			AbilitySystemComponent->SetRemoveAbilityOnEnd(AbilityHandle);
		}

		UBaseAbilitySystemComponent* BaseAsc = CastChecked<UBaseAbilitySystemComponent>(AbilitySystemComponent);
		for (FGrantedHandlesData& SetHandle : ActorExtensions->AbilitySetHandles)
		{
			SetHandle.TakeFromAbilitySystem(BaseAsc);
		}
	}

	ActiveData.ActiveExtensions.Remove(Actor);
}

UActorComponent* UGameFeatureAction_AddAbilities::FindOrAddComponentForActor(
	UClass* ComponentType, const AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry,
	FPerContextData& ActiveData) const
{
	UActorComponent* Component = Actor->FindComponentByClass(ComponentType);

	bool bMakeComponentRequest = (Component == nullptr);
	if (Component)
	{
		// Check to see if this component was created from a different `UGameFrameworkComponentManager` request.
		// `Native` is what `CreationMethod` defaults to for dynamically added components.
		if (Component->CreationMethod == EComponentCreationMethod::Native)
		{
			// Attempt to tell the difference between a true native component and one created by the GameFrameworkComponent system.
			// If it is from the UGameFrameworkComponentManager, then we need to make another request (requests are ref counted).
			const UObject* ComponentArchetype = Component->GetArchetype();
			bMakeComponentRequest = ComponentArchetype->HasAnyFlags(RF_ClassDefaultObject);
		}
	}

	if (bMakeComponentRequest)
	{
		const UWorld* World = Actor->GetWorld();
		const UGameInstance* GameInstance = World->GetGameInstance();

		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<
			UGameFrameworkComponentManager>(GameInstance))
		{
			const TSharedPtr<FComponentRequestHandle> RequestHandle = ComponentMan->AddComponentRequest(
				AbilitiesEntry.ActorClass, ComponentType);
			ActiveData.ComponentRequests.Add(RequestHandle);
		}

		if (!Component)
		{
			Component = Actor->FindComponentByClass(ComponentType);
			ensureAlways(Component);
		}
	}

	return Component;
}

#undef LOCTEXT_NAMESPACE
