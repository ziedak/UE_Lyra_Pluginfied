#include "BaseGameState.h"
#include "AbilitySystemComponent.h"
#include "Experience/ExperienceManagerComponent.h"
#include "GameFramework/PlayerState.h"
#include "MessageRuntime/GameplayMessageSubsystem.h"
#include "MessageVerb/VerbMessage.h"
#include "Net/UnrealNetwork.h"
#include "Net/NetPushModelHelpers.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGameState)
extern ENGINE_API float GAverageFPS;

ABaseGameState::ABaseGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UAbilitySystemComponent>(
		this, "AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ExperienceManagerComponent = CreateDefaultSubobject<UExperienceManagerComponent>("ExperienceManagerComponent");
}

void ABaseGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(/*Owner=*/ this, /*Avatar=*/ this);
}


void ABaseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	// DOREPLIFETIME(ThisClass, ServerFPS);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ServerFPS, SharedParams);
	// DOREPLIFETIME_CONDITION(ThisClass, RecorderPlayerState, COND_ReplayOnly);
}

void ABaseGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetServerFPS(GAverageFPS);
}

void ABaseGameState::SetServerFPS(const float NewServerFPS)
{
	if (GetLocalRole() != ROLE_Authority || ServerFPS == NewServerFPS)
	{
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ServerFPS, this);
	ServerFPS = NewServerFPS;
}

void ABaseGameState::MulticastMessageToClients_Unreliable_Implementation(const FVerbMessage Message)
{
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

void ABaseGameState::MulticastMessageToClients_Reliable_Implementation(const FVerbMessage Message)
{
	MulticastMessageToClients_Unreliable_Implementation(Message);
}

void ABaseGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	// Remove inactive and bots
	for (int32 i = PlayerArray.Num() - 1; i >= 0; i--)
	{
		auto PlayerState = PlayerArray[i];
		if (PlayerState && (PlayerState->IsABot() || PlayerState->IsInactive()))
		{
			RemovePlayerState(PlayerState);
		}
	}
}
