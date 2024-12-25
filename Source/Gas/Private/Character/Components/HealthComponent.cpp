#include "Character/Components/HealthComponent.h"

#include "Net/UnrealNetwork.h"
#include "MessageRuntime/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
// Game Includes
#include "Component/BaseAbilitySystemComponent.h"
#include "Attributes/HealthSet.h"
#include "Data/GasGameData.h"
#include "MessageVerb/VerbMessage.h"
#include "MessageVerb/VerbMessageHelpers.h"
#include "AssetManager/BaseAssetManager.h"
#include "Tags/BaseGameplayTags.h"
#include "Log/Loggger.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(HealthComponent)


// UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_GAS_ELIMINATION_MESSAGE, "GAS.Elimination.Message");


UHealthComponent::UHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  AbilitySystemComponent(nullptr),
	  HealthSet(nullptr),
	  DeathState(EDeathState::NotDead)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, DeathState);
}

void UHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void UHealthComponent::InitializeWithAbilitySystem(UBaseAbilitySystemComponent* InAsc)
{
	const AActor* Owner = GetOwner();
	check(Owner);
	if (AbilitySystemComponent)
	{
		LOG_ERROR(
			LogGAS,
			"HealthComponent: Health component for owner [%s] has already been initialized with an ability system.",
			*GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InAsc;
	if (!AbilitySystemComponent)
	{
		LOG_ERROR(
			LogGAS,
			"HealthComponent: Health component for owner [%s] failed to initialize with an ability system.",
			*GetNameSafe(Owner));
		return;
	}

	// Bind to health changed event on the ability system component to handle health changes. 
	HealthSet = AbilitySystemComponent->GetSet<UHealthSet>();
	if (!HealthSet)
	{
		LOG_ERROR(
			LogGAS,
			"HealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."
			, *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	HealthSet->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	HealthSet->OnMaxHealthChanged.AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	HealthSet->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOutOfHealth);

	// TEMP: Reset attributes to default values.  Eventually this will be driven by a spreadsheet. 
	AbilitySystemComponent->SetNumericAttributeBase(UHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());

	ClearGameplayTags();

	OnHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
	OnMaxHealthChanged.Broadcast(this, HealthSet->GetHealth(), HealthSet->GetHealth(), nullptr);
}

void UHealthComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (HealthSet)
	{
		HealthSet->OnHealthChanged.RemoveAll(this);
		HealthSet->OnMaxHealthChanged.RemoveAll(this);
		HealthSet->OnOutOfHealth.RemoveAll(this);
	}

	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float UHealthComponent::GetHealth() const { return (HealthSet ? HealthSet->GetHealth() : 0.0f); }

float UHealthComponent::GetHealthNormalized() const
{
	if (!HealthSet) { return 0.0f; }

	const float Health = GetHealth();
	const float MaxHealth = GetMaxHealth();

	return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
}

float UHealthComponent::GetMaxHealth() const { return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f); }

void UHealthComponent::StartDeath()
{
	if (DeathState != EDeathState::NotDead) { return; }

	DeathState = EDeathState::DeathStarted;

	// Set the death gameplay tags.
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(StatusTags::DEATH_DYING, 1);
		AbilitySystemComponent->SetLooseGameplayTagCount(StatusTags::DEATH_DEAD, 0);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);
	Owner->ForceNetUpdate();
}

void UHealthComponent::FinishDeath()
{
	if (DeathState != EDeathState::DeathStarted) { return; }

	DeathState = EDeathState::DeathFinished;

	if (AbilitySystemComponent) { AbilitySystemComponent->SetLooseGameplayTagCount(StatusTags::DEATH_DEAD, 1); }

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void UHealthComponent::DamageSelfDestruct(const bool bFellOutOfWorld)
{
	if (DeathState != EDeathState::NotDead || !AbilitySystemComponent) { return; }

	const TSubclassOf<UGameplayEffect> DamageGE = UBaseAssetManager::GetSubclass(
		UGasGameData::Get().DamageGameplayEffect_SetByCaller);
	if (!DamageGE)
	{
		ULOG_ERROR(
			LogGAS,
			"HealthComponent::DamageSelfDestruct : Failed to find damage gameplay effect for owner [%s].Unable to find gameplay effect [%s].",
			*GetNameSafe(GetOwner()), *UGasGameData::Get().DamageGameplayEffect_SetByCaller.GetAssetName());
		return;
	}

	const FGameplayEffectSpecHandle DamageEffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		DamageGE, 1.0f, AbilitySystemComponent->MakeEffectContext());
	if (!DamageEffectSpecHandle.IsValid())
	{
		ULOG_ERROR(
			LogGAS,
			"HealthComponent::DamageSelfDestruct : Failed to create damage gameplay effect spec for owner [%s].",
			*GetNameSafe(GetOwner()));
		return;
	}

	FGameplayEffectSpec* DamageEffectSpec = DamageEffectSpecHandle.Data.Get();
	if (!DamageEffectSpec)
	{
		ULOG_ERROR(
			LogGAS,
			"HealthComponent::DamageSelfDestruct : Failed to create damage gameplay effect spec for owner [%s].Unable to make outgoing spec for [%s].",
			*GetNameSafe(GetOwner()), *GetNameSafe(DamageGE));
		return;
	}

	DamageEffectSpec->AddDynamicAssetTag(BaseGameplayTags::DAMAGE_SELF_DESTRUCT);

	if (bFellOutOfWorld) { DamageEffectSpec->AddDynamicAssetTag(BaseGameplayTags::FELL_OUT_OF_WORLD); }

	const float DamageMagnitude = GetMaxHealth();
	DamageEffectSpec->SetSetByCallerMagnitude(SetByCallerTags::DAMAGE, DamageMagnitude);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageEffectSpec);
}

// Clears the death state and any gameplay tags related to death.
void UHealthComponent::ClearGameplayTags() const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(StatusTags::DEATH_DYING, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(StatusTags::DEATH_DEAD, 0);
	}
}

void UHealthComponent::HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser,
                                           const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude,
                                           const float OldValue, const float NewValue)
{
	OnHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UHealthComponent::HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser,
                                              const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude,
                                              const float OldValue, const float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void UHealthComponent::HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser,
                                         const FGameplayEffectSpec* DamageEffectSpec, const float DamageMagnitude,
                                         float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (!AbilitySystemComponent || !DamageEffectSpec) { return; }

	//Send the "GameplayEvent.Death" gameplay event through the owner's ability system.  This can be used to trigger a death gameplay ability.
	//This is done in a prediction window to ensure the event is sent in the correct order.
	//This is also done on the server only to avoid spamming the ability system.
	{
		FGameplayEventData Payload;
		Payload.EventTag = GameplayEventTags::DEATH;
		Payload.Instigator = DamageInstigator;
		Payload.Target = AbilitySystemComponent->GetAvatarActor(); //GetOwner();
		Payload.OptionalObject = DamageEffectSpec->Def;
		Payload.ContextHandle = DamageEffectSpec->GetEffectContext();
		Payload.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
		Payload.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
		Payload.EventMagnitude = DamageMagnitude;
		FScopedPredictionWindow ScopedPredictionWindow(AbilitySystemComponent, true);
		AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
	}


	// Send a standardized verb message that other systems can observe and react to.
	// This is a simple example of how to send a message when a player is eliminated.
	// This could be used to trigger a kill feed, play a sound, or any other gameplay effect.
	// This is done in a prediction window to ensure the event is sent in the correct order.
	// This is also done on the server only to avoid spamming the message system.
	{
		FVerbMessage VerbMessage;
		VerbMessage.Verb = BaseGameplayTags::ELIMINATION_MESSAGE;
		VerbMessage.Instigator = DamageInstigator;
		VerbMessage.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
		VerbMessage.Target = UVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
		VerbMessage.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
		//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
		//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(VerbMessage.Verb, VerbMessage);
	}

	//@TODO: assist messages (could compute from damage dealt elsewhere)?

#endif // #if WITH_SERVER_CODE
}

// RepNotify for DeathState property.
void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	const EDeathState NewDeathState = DeathState;
	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		//the server is trying to set us back, but we have already predicted past the server state
		ULOG_WARNING(LogGAS, "HealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s].",
		             (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == EDeathState::NotDead)
	{
		if (NewDeathState == EDeathState::DeathStarted) { StartDeath(); }
		else if (NewDeathState == EDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			ULOG_ERROR(LogGAS, "HealthComponent: Invalid death transition [%d] -> [%d] for owner [%s].",
			           (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EDeathState::DeathStarted && NewDeathState == EDeathState::DeathFinished)
	{
		FinishDeath();
	}
	else
	{
		ULOG_ERROR(LogGAS, "HealthComponent: Invalid death transition [%d] -> [%d] for owner [%s].",
		           (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
	}


	ensureMsgf((DeathState == NewDeathState),
	           TEXT("HealthComponent: Death transition failed [%d] -> [%d] for owner [%s]."),
	           static_cast<uint8>(OldDeathState),
	           static_cast<uint8>(NewDeathState), *GetNameSafe(GetOwner()));
}
