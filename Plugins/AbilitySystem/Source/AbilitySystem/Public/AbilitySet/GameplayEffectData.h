#pragma once

#include "GameplayEffectData.generated.h"

class UGameplayEffect;


/**
 * FGameplayEffectData
 *
 *	Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FGameplayEffectData
{
	GENERATED_BODY()

	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};
