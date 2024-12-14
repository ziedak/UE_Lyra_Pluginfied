#include "Abilities/GameplayAbility_Jump.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Tags/BaseGameplayTags.h"
#include "Log/Loggger.h"

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
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}
	//TODO: implement this
	//const ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(ActorInfo->AvatarActor.Get());
	//if (!LyraCharacter || !LyraCharacter->CanJump())
	//{
	//	return false;
	//}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

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
	/*ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo();
		if (LyraCharacter)
		{
			if (LyraCharacter->IsLocallyControlled() && !LyraCharacter->bPressedJump)
			{
				LyraCharacter->UnCrouch();
				LyraCharacter->Jump();
			}
		}*/
}

// TODO: implement this
void UGameplayAbility_Jump::StopJump()
{
	/*ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo();
	if ( LyraCharacter )
	{
		if (LyraCharacter->IsLocallyControlled() && LyraCharacter->bPressedJump)
		{
			LyraCharacter->StopJumping();
		}
	}*/
}
