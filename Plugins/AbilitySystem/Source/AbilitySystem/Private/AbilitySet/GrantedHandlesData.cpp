#include "AbilitySet/GrantedHandlesData.h"
#include "Component/BaseAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GrantedHandlesData)

void FGrantedHandlesData::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FGrantedHandlesData::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FGrantedHandlesData::AddAttributeSet(UAttributeSet* Set)
{
	if (Set)
	{
		GrantedAttributeSets.Add(Set);
	}
}

void FGrantedHandlesData::TakeFromAbilitySystem(UBaseAbilitySystemComponent* Asc)
{
	check(Asc);

	if (!Asc->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// Clear all granted abilities
	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			Asc->ClearAbility(Handle);
		}
	}

	// Remove all granted effects
	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			Asc->RemoveActiveGameplayEffect(Handle);
		}
	}

	// Remove all granted attribute sets
	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		Asc->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}
