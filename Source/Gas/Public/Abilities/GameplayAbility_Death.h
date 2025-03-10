#pragma once
#include "GamePlayAbility_Base.h"
#include "GameplayAbility_Death.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;


/**
 * UGameplayAbility_Death
 *
 *	Gameplay ability used for handling death.
 *	Ability is activated automatically via the "GameplayEvent.Death" ability trigger tag.
 */
UCLASS(Abstract)
class UGameplayAbility_Death : public UGamePlayAbility_Base
{
	GENERATED_BODY()

public:
	UGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

	// Starts the death sequence.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility|Ability")
	void StartDeath() const;

	// Finishes the death sequence.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility|Ability")
	void FinishDeath() const;

	// If enabled, the ability will automatically call StartDeath.  FinishDeath is always called when the ability ends if the death was started.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility|Death")
	bool bAutoStartDeath;
};