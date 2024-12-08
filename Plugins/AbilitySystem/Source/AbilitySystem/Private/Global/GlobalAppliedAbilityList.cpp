#include "Global/GlobalAppliedAbilityList.h"

#include "Component/BaseAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GlobalAppliedAbilityList)

void FGlobalAppliedAbilityList::AddToAsc(const TSubclassOf<UGameplayAbility>& Ability, UBaseAbilitySystemComponent* Asc)
{
	if (Handles.Find(Asc))
	{
		RemoveFromAsc(Asc);
	}

	UGameplayAbility* AbilityCDO = Ability->GetDefaultObject<UGameplayAbility>();
	const FGameplayAbilitySpec AbilitySpec(AbilityCDO);
	const FGameplayAbilitySpecHandle AbilitySpecHandle = Asc->GiveAbility(AbilitySpec);
	Handles.Add(Asc, AbilitySpecHandle);
}

void FGlobalAppliedAbilityList::RemoveFromAsc(UBaseAbilitySystemComponent* ASC)
{
	if (const FGameplayAbilitySpecHandle* SpecHandle = Handles.Find(ASC))
	{
		ASC->ClearAbility(*SpecHandle);
		Handles.Remove(ASC);
	}
}

void FGlobalAppliedAbilityList::RemoveFromAll()
{
	for (auto& Pair : Handles)
	{
		if (UBaseAbilitySystemComponent* Asc = Pair.Key.Get())
		{
			Asc->ClearAbility(Pair.Value);
		}
	}
	Handles.Empty();
}
