#include "Abilities/GameplayAbility_Reset.h"

#include "Character/BaseCharacter.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "MessageRuntime/GameplayMessageSubsystem.h"
#include "Tags/BaseGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbility_Reset)

UGameplayAbility_Reset::UGameplayAbility_Reset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = GameplayEventTags::REQUEST_RESET;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGameplayAbility_Reset::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	check(ActorInfo);
	UBaseAbilitySystemComponent* BaseAsc = CastChecked<UBaseAbilitySystemComponent>(
		ActorInfo->AbilitySystemComponent.Get());
	check(BaseAsc);

	FGameplayTagContainer AbilityTagsToIgnore;
	AbilityTagsToIgnore.AddTag(AbilityTags::BEHAVIOR_SURVIVES_DEATH);

	// Cancel all abilities and block other from activating/starting
	BaseAsc->CancelAbilities(nullptr, &AbilityTagsToIgnore, this);
	SetCanBeCanceled(false);

	//@TODO:Implement this
	//Execute the reset from the ABaseCharacter
	 if (ABaseCharacter* BaseChar = Cast<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get()))
	 	BaseChar->Reset();


	// Let others know a reset has occurred
	FPlayerResetMessage Message;
	Message.OwnerPlayerState = CurrentActorInfo->OwnerActor.Get();
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(GameplayEventTags::RESET, Message);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	constexpr bool bReplicateEndAbility = true;
	constexpr bool bWasCanceled = false;

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCanceled);

}
