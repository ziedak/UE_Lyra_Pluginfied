// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/PawnExtensionComponent.h"

#include "Component/BaseAbilitySystemComponent.h"
#include "Data/GasPawnData.h"


#include "GameFramework/Pawn.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"

#include "Tags/BaseGameplayTags.h"
#include "Tags/CoreTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PawnExtensionComponent)
class FLifetimeProperty;
class UActorComponent;

const FName UPawnExtensionComponent::Name_ActorFeatureName("PawnExtension");

UPawnExtensionComponent::UPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  PawnData(nullptr),
	  AbilitySystemComponent(nullptr)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

#pragma region IGameFrameworkInitStateInterface

void UPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPawnExtensionComponent, PawnData);
}

bool UPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager,
                                                 FGameplayTag CurrentState,
                                                 FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();
	// As long as we are on a valid pawn, we count as spawned
	if (!CurrentState.IsValid() && DesiredState == InitStateTags::SPAWNED && Pawn) { return true; }

	if (CurrentState == InitStateTags::SPAWNED && DesiredState == InitStateTags::DATA_AVAILABLE)
	{
		// Pawn data is required.
		if (!PawnData) { return false; }


		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		// Check for being possessed by a controller.
		if ((bHasAuthority || bIsLocallyControlled) && !GetController<AController>()) { return false; }

		return true;
	}
	if (CurrentState == InitStateTags::DATA_AVAILABLE && DesiredState == InitStateTags::DATA_INITIALIZED)
	{
		// Transition to initialize if all features have their data available
		return Manager->HaveAllFeaturesReachedInitState(Pawn, InitStateTags::DATA_AVAILABLE);
	}
	if (CurrentState == InitStateTags::DATA_INITIALIZED && DesiredState == InitStateTags::GAMEPLAY_READY)
	{
		return true;
	}

	return false;
}

void UPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager,
                                                    FGameplayTag CurrentState,
                                                    FGameplayTag DesiredState)
{
	if (DesiredState != InitStateTags::DATA_INITIALIZED) { return; }

	IGameFrameworkInitStateInterface::HandleChangeInitState(Manager, CurrentState, DesiredState);
	// This is currently all handled by other components listening to this state change
}

void UPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);

	// If another feature is now in DataAvailable, see if we should transition to DataInitialized
	if (Params.FeatureName == Name_ActorFeatureName) { return; }

	if (Params.FeatureState != InitStateTags::DATA_AVAILABLE) { return; }

	CheckDefaultInitialization();
}

void UPawnExtensionComponent::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();
	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = {
		InitStateTags::SPAWNED,
		InitStateTags::DATA_AVAILABLE,
		InitStateTags::DATA_INITIALIZED,
		InitStateTags::GAMEPLAY_READY
	};

	// This will try to progress from spawned (which is only set in BeginPlay)
	// through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

#pragma endregion

void UPawnExtensionComponent::SetPawnData(const UGasPawnData* InPawnData)
{
	check(InPawnData);
	// Only the server should be setting the pawn data
	APawn* Pawn = GetPawnChecked<APawn>();

	if (Pawn && Pawn->GetLocalRole() != ROLE_Authority) { return; }

	// If we already have a PawnData, we should not be setting it again
	if (PawnData)
	{
		UE_LOG(LogTemp, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."),
		       *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	Pawn->ForceNetUpdate();
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::OnRep_PawnData() { CheckDefaultInitialization(); }

void UPawnExtensionComponent::InitializeAbilitySystem(UBaseAbilitySystemComponent* InAsc, AActor* InOwnerActor)
{
	check(InAsc);
	check(InOwnerActor);

	// The ability system component hasn't changed.
	if (AbilitySystemComponent == InAsc) { return; }

	// Clean up the old ability system component.
	if (AbilitySystemComponent) { UninitializeAbilitySystem(); }

	APawn* Pawn = GetPawnChecked<APawn>();
	const AActor* ExistingAvatar = InAsc->GetAvatarActor();

	UE_LOG(LogTemp, Verbose,
	       TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "),
	       *GetNameSafe(InAsc),
	       *GetNameSafe(Pawn),
	       *GetNameSafe(InOwnerActor),
	       *GetNameSafe(ExistingAvatar));


	if (ExistingAvatar && ExistingAvatar != Pawn)
	{
		UE_LOG(LogTemp, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// There is already a pawn acting as the avatar of ASC , so we need to kick it out
		// This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
		ensure(!ExistingAvatar->HasAuthority());

		if (UPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InAsc;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (ensure(PawnData)) { InAsc->SetTagRelationshipMapping(PawnData->TagRelationshipMapping); }

	OnAbilitySystemInitialized.Broadcast();
}

void UPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(
	const FSimpleMulticastDelegate::FDelegate& Delegate)
{
	// If the delegate is already bound to this object, don't bind it again
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	// If the ability system is already initialized, call the delegate immediately
	if (AbilitySystemComponent) { Delegate.Execute(); }
}

void UPawnExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent) { return; }
	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(AbilityTags::BEHAVIOR_SURVIVES_DEATH);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
		AbilitySystemComponent->GetOwnerActor()
			? AbilitySystemComponent->SetAvatarActor(nullptr)
			: AbilitySystemComponent->ClearActorInfo();

		OnAbilitySystemUninitialized.Broadcast();
	}
	AbilitySystemComponent = nullptr;
}

void UPawnExtensionComponent::OnAbilitySystemUninitialized_Register(const FSimpleMulticastDelegate::FDelegate& Delegate)
{
	if (OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject())) { return; }

	OnAbilitySystemUninitialized.Add(Delegate);
}

void UPawnExtensionComponent::HandleControllerChanged()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
		AbilitySystemComponent->GetOwnerActor()
			? AbilitySystemComponent->RefreshAbilityActorInfo()
			: UninitializeAbilitySystem();
	}

	CheckDefaultInitialization();
}

void UPawnExtensionComponent::HandlePlayerStateReplicated() { CheckDefaultInitialization(); }

void UPawnExtensionComponent::SetupPlayerInputComponent() { CheckDefaultInitialization(); }

void UPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();
	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf(Pawn,
	                 TEXT("PawnExtensionComponent on [%s] can only be added to Pawn actors."),
	                 *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> PawnExtensions;
	Pawn->GetComponents(StaticClass(),/*outComponents*/PawnExtensions);
	ensureMsgf(PawnExtensions.Num() == 1,
	           TEXT("PawnExtensionComponent on [%s] can only be added to Pawn actors once."),
	           *GetNameSafe(GetOwner()));

	// Register with the init state system early, 
	// this will only work if this is a game world
	RegisterInitStateFeature();
}

void UPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();
	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(InitStateTags::SPAWNED));
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}
