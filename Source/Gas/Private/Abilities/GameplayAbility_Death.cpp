#include "Abilities/GameplayAbility_Death.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Tags/BaseGameplayTags.h"
#include "Log/Loggger.h"
//TODO : verify this
// #include "Trace/Trace.inl"
#include "Character/Components/HealthComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbility_Death)

UGameplayAbility_Death::UGameplayAbility_Death(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	bAutoStartDeath = true;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		//Add the ability trigger  tag as default tag for this ability
		//This will allow the ability to be triggered by the specified tag

		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = GameplayEventTags::DEATH;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);
	UBaseAbilitySystemComponent* BaseASC = Cast<UBaseAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	if (!BaseASC)
	{
		ULOG_ERROR(
			LogGAS, "UGameplayAbility_Death::ActivateAbility called with invalid AbilitySystemComponent. Actor: %s",
			*ActorInfo->OwnerActor->GetName());
		return;
	}

	FGameplayTagContainer AbilityTagsToIgnore;
	AbilityTagsToIgnore.AddTag(AbilityTags::BEHAVIOR_SURVIVES_DEATH);

	// Cancel all abilities and block other from activating/starting
	BaseASC->CancelAbilities(nullptr, &AbilityTagsToIgnore, this);

	SetCanBeCanceled(false);

	if (!ChangeActivationGroup(EAbilityActivationGroup::Exclusive_Blocking))
		ULOG_ERROR(
		LogGAS, "UGameplayAbility_Death::ActivateAbility Ability [%s] failed to change activation group to blocking.",
		*GetName());

	if (bAutoStartDeath)
		StartDeath();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                        bool bReplicateEndAbility,
                                        bool bWasCancelled)
{
	check(ActorInfo);

	// Always try to finish the death when the ability ends in case the ability doesn't.
	// This won't do anything if the death hasn't been started.
	FinishDeath();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//TODO : Implement UBaseHealthComponent
void UGameplayAbility_Death::StartDeath() const
{
	UHealthComponent* HealthComponent = UHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo());
	if (!HealthComponent)
	{
		ULOG_ERROR(LogGAS, "UGameplayAbility_Death::StartDeath called with invalid HealthComponent. Actor: %s",
		                 *GetAvatarActorFromActorInfo()->GetName());
		return;
	}
	if (HealthComponent->GetDeathState() != EDeathState::NotDead)
	{
		ULOG_WARNING(
			LogGAS, "UGameplayAbility_Death::StartDeath called when the actor is already dead. Actor: %s",
			*GetAvatarActorFromActorInfo()->GetName());
		return;
	}
	HealthComponent->StartDeath();
}


//TODO : Implement UBaseHealthComponent
void UGameplayAbility_Death::FinishDeath() const
{
	UHealthComponent* HealthComponent = UHealthComponent::FindHealthComponent(GetAvatarActorFromActorInfo());
	if (!HealthComponent)
	{
		ULOG_ERROR(LogGAS, "UGameplayAbility_Death::StartDeath called with invalid HealthComponent. Actor: %s",
		                 *GetAvatarActorFromActorInfo()->GetName());
		return;
	}
	if (HealthComponent->GetDeathState() != EDeathState::DeathStarted)
	{
		ULOG_WARNING(LogGAS, "UGameplayAbility_Death::FinishDeath called when the actor is not dead. Actor: %s",
		                   *GetAvatarActorFromActorInfo()->GetName());
		return;
	}

	HealthComponent->FinishDeath();
}
