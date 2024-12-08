#pragma once

#include "Templates/SubclassOf.h"
#include "ActiveGameplayEffectHandle.h"
#include "GlobalAppliedEffectList.generated.h"

class UGameplayEffect;
class UBaseAbilitySystemComponent;
struct FActiveGameplayEffectHandle;


USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UBaseAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToAsc(const TSubclassOf<UGameplayEffect>& Effect, UBaseAbilitySystemComponent* Asc);
	void RemoveFromAsc(UBaseAbilitySystemComponent* Asc);
	void RemoveFromAll();
};
