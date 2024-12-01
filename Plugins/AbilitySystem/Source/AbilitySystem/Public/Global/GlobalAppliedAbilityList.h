#pragma once

#include "Templates/SubclassOf.h"
#include "GameplayAbilitySpecHandle.h"
#include "GlobalAppliedAbilityList.generated.h"


class UGameplayAbility;
class UBaseAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;

USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UBaseAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToAsc(const TSubclassOf<UGameplayAbility>& Ability, UBaseAbilitySystemComponent* Asc);
	void RemoveFromAsc(UBaseAbilitySystemComponent* ASC);
	void RemoveFromAll();
};