#pragma once
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"


#include "AttributeSetData.generated.h"

class UAttributeSet;
/**
 * FAttributeSetData
 *
 *	Data used by the ability set to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct FAttributeSetData
{
	GENERATED_BODY()

	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;
};
