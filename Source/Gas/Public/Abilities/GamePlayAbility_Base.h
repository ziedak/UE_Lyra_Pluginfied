// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ability/Abilities/BaseGameplayAbility.h"

#include "Character/Components/HeroComponent.h"

#include "GamePlayAbility_Base.generated.h"

class UCustomCameraMode;
/**
 * 
 */
UCLASS()
class GAS_API UGamePlayAbility_Base : public UBaseGameplayAbility
{
	GENERATED_BODY()

public:
	UGamePlayAbility_Base(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Current camera mode set by the ability.
	TSubclassOf<UCustomCameraMode> ActiveCameraMode;

	UHeroComponent* GetHeroComponentFromActorInfo() const;

	// Sets the ability's camera mode.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	void SetCameraMode(const TSubclassOf<UCustomCameraMode>& CameraMode);

	// Clears the ability's camera mode.  Automatically called if needed when the ability ends.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	void ClearCameraMode();

protected:
	virtual void EndAbility(FGameplayAbilitySpecHandle Handle,
	                        const FGameplayAbilityActorInfo* ActorInfo,
	                        FGameplayAbilityActivationInfo ActivationInfo,
	                        bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
};
