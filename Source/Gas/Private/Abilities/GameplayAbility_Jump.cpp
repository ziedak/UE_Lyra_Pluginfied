#include "Abilities/GameplayAbility_Jump.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Tags/BaseGameplayTags.h"
#include "Log/Log.h"
#include <Character/BaseCharacter.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbility_Jump)

UGameplayAbility_Jump::UGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGameplayAbility_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) { return false; }

	const ABaseCharacter* Character = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !Character->CanJump())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) { return false; }

	return true;
}

void UGameplayAbility_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Stop jumping in case the ability blueprint doesn't call it.
	StopJump();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// TODO: implement this
void UGameplayAbility_Jump::StartJump()
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(GetCharacterFromActorInfo());
	if (Character && Character->IsLocallyControlled() && !Character->bPressedJump)
	{
		Character->UnCrouch();
		Character->Jump();
	}
}

// TODO: implement this
void UGameplayAbility_Jump::StopJump()
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(GetCharacterFromActorInfo());

	if (Character && Character->IsLocallyControlled() && Character->bPressedJump)
	{
		Character->StopJumping();
	}
}