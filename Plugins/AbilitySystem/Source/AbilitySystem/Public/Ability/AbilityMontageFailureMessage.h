#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMontage.h"

#include "AbilityMontageFailureMessage.generated.h"
/** Failure reason that can be used to play an animation montage when a failure occurs */
USTRUCT(BlueprintType)
struct FAbilityMontageFailureMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// All the reasons why this ability has failed
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};
