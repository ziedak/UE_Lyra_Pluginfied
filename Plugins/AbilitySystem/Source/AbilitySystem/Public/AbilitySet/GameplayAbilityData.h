#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
//#include "UObject/NoExportTypes.h"
#include "GameplayAbilityData.generated.h"

class UBaseGameplayAbility;
// struct FGameplayTag;
/**
 * FGameplayAbilityData
 *
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct ABILITYSYSTEM_API FGameplayAbilityData
{
	GENERATED_BODY()

public:

	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UBaseGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};