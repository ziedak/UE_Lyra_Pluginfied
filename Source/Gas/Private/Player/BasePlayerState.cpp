// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BasePlayerState.h"

#include "Ability/BaseAbilitySet.h"
#include "Attributes/CombatSet.h"
#include "Attributes/HealthSet.h"
#include "Character/Components/PawnExtensionComponent.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Core/GGameMode.h"
#include "Data/GasPawnData.h"
#include "Experience/ExperienceManagerComponent.h"
#include "MessageRuntime/GameplayMessageSubsystem.h"
#include "MessageVerb/VerbMessage.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/BaseGameMode.h"
#include "Log/Log.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BasePlayerState)
const FName ABasePlayerState::NAME_BaseAbilityReady("BaseAbilitiesReady");

ABasePlayerState::ABasePlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
	, MyPlayerConnectionType(EPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UBaseAbilitySystemComponent>(
		this, "AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// These attribute sets will be detected by AbilitySystemComponent::InitializeComponent. Keeping a reference so that the sets don't get garbage collected before that.
	HealthSet = CreateDefaultSubobject<UHealthSet>("HealthSet");
	CombatSet = CreateDefaultSubobject<UCombatSet>("CombatSet");

	// AbilitySystemComponent needs to be updated at a high frequency.
	NetUpdateFrequency = 100.0f;
}


void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)

	SharedParams.Condition = COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);
}

UAbilitySystemComponent* ABasePlayerState::GetAbilitySystemComponent() const { return GetBaseAbilitySystemComponent(); }

void ABasePlayerState::SetPawnData(const UGasPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority) { return; }

	if (PawnData)
	{
		UE_LOG(LogGAS, Error,
		       TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."),
		       *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	// The MARK_PROPERTY_DIRTY_FROM_NAME macro is used to mark a property as "dirty" to ensure it gets
	// replicated to clients in a networked game. This is particularly important in Unreal Engine's
	// networking model, where only properties that have changed (i.e., marked as dirty) are sent over
	// the network to keep bandwidth usage efficient.
	//==> By marking PawnData as dirty, the Unreal Engine's replication system knows that this property has changed
	// and needs to be updated on all clients. 
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UBaseAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet) { AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr); }
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_BaseAbilityReady);

	ForceNetUpdate();
}

#pragma region Actor interface

void ABasePlayerState::PreInitializeComponents() { Super::PreInitializeComponents(); }

void ABasePlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());


	//==> The OnExperienceLoaded function is called when the experience is loaded.
	const UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_Client) { return; }

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	check(GameState);

	UExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<
		UExperienceManagerComponent>();
	check(ExperienceComponent);

	ExperienceComponent->CallOrRegister_OnExperienceLoaded(
		FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}
#pragma endregion

void ABasePlayerState::OnExperienceLoaded(const UExperienceDefinition_DA* Experience)
{
	//==> The OnExperienceLoaded function is called when the experience is loaded.
	// This function is used to set the player state's PawnData property to the default pawn data
	// for the current experience.
	const AGGameMode* GameMode = GetWorld()->GetAuthGameMode<AGGameMode>();
	check(GameMode);

	const auto NewPawnData = GameMode->GetPawnDataForController(GetOwningController());
	if (!NewPawnData)
	{
		ULOG_ERROR(LogGAS, "Unable to find PawnData to initialize player state [%s]!", *GetNameSafe(this));
		return;
	}

	SetPawnData(NewPawnData);
}


#pragma region APlayerState interface
void ABasePlayerState::Reset() { Super::Reset(); }

void ABasePlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

void ABasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	//@TODO: Copy stats
}

void ABasePlayerState::OnDeactivated()
{
	//==> The OnDeactivated function is called when the player state is deactivated.
	// This can happen when a player disconnects from the game or when the player is removed
	// from the game for some other reason.

	Super::OnDeactivated();
	bool bDestroyDeactivatedPlayerState;
	//==> The GetPlayerConnectionType function is used to
	// determine the type of connection the player has to the game.
	switch (GetPlayerConnectionType())
	{
	case EPlayerConnectionType::Player:
	case EPlayerConnectionType::InactivePlayer:
		//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
		// (e.g., for long running servers where they might build up if lots of players cycle through)
		bDestroyDeactivatedPlayerState = true;
		break;
	default:
		bDestroyDeactivatedPlayerState = true;
		break;
	}

	SetPlayerConnectionType(EPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState) { Destroy(); }
}

void ABasePlayerState::OnReactivated()
{
	Super::OnReactivated();
	if (GetPlayerConnectionType() == EPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EPlayerConnectionType::Player);
	}
}
#pragma endregion

void ABasePlayerState::SetPlayerConnectionType(const EPlayerConnectionType NewType)
{
	if (MyPlayerConnectionType == NewType) { return; }
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MyPlayerConnectionType, this);
	MyPlayerConnectionType = NewType;
}

void ABasePlayerState::OnRep_PawnData() {}

void ABasePlayerState::ClientBroadcastMessage_Implementation(const FVerbMessage Message)
{
	// This check is needed to prevent running the action when in standalone mode
	if (GetNetMode() == NM_Client) { UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message); }
}


void ABasePlayerState::SetReplicatedViewRotation(const FRotator& NewRot)
{
	if (ReplicatedViewRotation == NewRot) { return; }
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);
	ReplicatedViewRotation = NewRot;
}
