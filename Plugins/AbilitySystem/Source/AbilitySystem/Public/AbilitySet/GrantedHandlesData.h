#pragma once

#include "GrantedHandlesData.generated.h"

class UBaseAbilitySystemComponent;
class UAttributeSet;
struct FGameplayAbilitySpecHandle;
struct FActiveGameplayEffectHandle;


/**
 * FGrantedHandlesData
 *
 *	Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct ABILITYSYSTEM_API FGrantedHandlesData
{
	GENERATED_BODY()

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	void TakeFromAbilitySystem(UBaseAbilitySystemComponent* BaseASC);

protected:
	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};
