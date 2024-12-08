#include "Global/GlobalAppliedEffectList.h"

#include "Component/BaseAbilitySystemComponent.h"

//#include UE_INLINE_GENERATED_CPP_BY_NAME(FGlobalAppliedEffectList)

void FGlobalAppliedEffectList::AddToAsc(const TSubclassOf<UGameplayEffect>& Effect, UBaseAbilitySystemComponent* Asc)
{
	if (Handles.Find(Asc))
	{
		RemoveFromAsc(Asc);
	}

	const UGameplayEffect* GameplayEffectCDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle GameplayEffectHandle = Asc->ApplyGameplayEffectToSelf(
		GameplayEffectCDO, /*Level=*/ 1, Asc->MakeEffectContext());
	Handles.Add(Asc, GameplayEffectHandle);
}

void FGlobalAppliedEffectList::RemoveFromAsc(UBaseAbilitySystemComponent* Asc)
{
	if (FActiveGameplayEffectHandle* EffectHandle = Handles.Find(Asc))
	{
		Asc->RemoveActiveGameplayEffect(*EffectHandle);
		Handles.Remove(Asc);
	}
}

void FGlobalAppliedEffectList::RemoveFromAll()
{
	for (auto& Pair : Handles)
	{
		UBaseAbilitySystemComponent* Asc = Pair.Key.Get();
		if (Asc)
		{
			Asc->RemoveActiveGameplayEffect(Pair.Value);
		}
	}
	Handles.Empty();
}
