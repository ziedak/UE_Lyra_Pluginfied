// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "BaseAbilitySystemGlobals.generated.h"

/**
* Must be declared in DefaultEngine.ini like so:
* [/Script/GameplayAbilities.AbilitySystemGlobals]
* AbilitySystemGlobalsClassName=/Script/<GameName>.BaseAbilitySystemGlobals
 */
UCLASS()
class ABILITYSYSTEM_API UBaseAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	/**
	* Cache commonly used tags here. This has the benefit of one place to set the tag FName in case tag names change and
	* the function call into UBaseAbilitySystemGlobals::GSGet() is cheaper than calling FGameplayTag::RequestGameplayTag().
	* Classes can access them by UBaseAbilitySystemGlobals::GetBase().DeadTag
	* We're not using this in this sample project (classes are manually caching in their constructors), but it's here as a reference.
	*/

	UPROPERTY()
	FGameplayTag DeadTag;

	UPROPERTY()
	FGameplayTag KnockedDownTag;

	UPROPERTY()
	FGameplayTag InteractingTag;

	UPROPERTY()
	FGameplayTag InteractingRemovalTag;

	static UBaseAbilitySystemGlobals& GetBase()
	{
		return dynamic_cast<UBaseAbilitySystemGlobals&>(Get());
	}

	/** Should allocate a project specific GameplayEffectContext struct. Caller is responsible for deallocation */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;

	virtual void InitGlobalTags() override;
};
