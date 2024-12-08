// Fill out your copyright notice in the Description page of Project Settings.

#include "Global/BaseAbilitySystemGlobals.h"

#include "Global/BaseGameplayEffectContext.h"
#include "Tags/BaseGameplayTags.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseAbilitySystemGlobals)

FGameplayEffectContext* UBaseAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FBaseGameplayEffectContext();
}

void UBaseAbilitySystemGlobals::InitGlobalTags()
{
	Super::InitGlobalTags();

	DeadTag = StateTags::DEAD;
	KnockedDownTag = StateTags::KNOCKED_DOWN;
	InteractingTag = StateTags::INTERACTING;
	InteractingRemovalTag = StateTags::INTERACTING_REMOVAL;
}
