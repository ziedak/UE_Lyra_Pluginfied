// Fill out your copyright notice in the Description page of Project Settings.

#pragma once



#include "GameFramework/Actor.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealth_DeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHealth_AttributeChanged, UHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

class UBaseAbilitySystemComponent;
class UHealthSet;
struct FGameplayEffectSpec;

/**
 * EDeathState
 *
 *	Defines current state of death.
 */
UENUM(BlueprintType)
enum class EDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};


/**
 * 	An actor component used to handle anything related to health.
 */
UCLASS()
class GAS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()
public :
	UHealthComponent(const FObjectInitializer& ObjectInitializer);

	//Returns the current health Component
	UFUNCTION(BlueprintPure, Category = "Health")
	static UHealthComponent* FindHealthComponent(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UHealthComponent>() : nullptr;
	}

	//Initializes the health component
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitializeWithAbilitySystem(UBaseAbilitySystemComponent* InAbilitySystemComponent);
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	 void UninitializeFromAbilitySystem() ;
	
	//Returns the current health
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const;

	// Returns the current health in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthNormalized() const;
	
	//Returns the maximum health
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	//Returns the current death state
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE EDeathState GetDeathState() const{return DeathState;};

	//Returns true if the actor is dead
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	FORCEINLINE bool IsDeadOrDying() const { return DeathState != EDeathState::NotDead; };

	// Begins the death sequence for the owner.
	virtual void StartDeath();

	// Ends the death sequence for the owner.
	virtual void FinishDeath();

	// Applies enough damage to kill the owner.
	virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

public:

	// Delegate fired when the health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChanged OnHealthChanged;

	// Delegate fired when the max health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FHealth_AttributeChanged OnMaxHealthChanged;

	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable)
	FHealth_DeathEvent OnDeathStarted;

	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable)
	FHealth_DeathEvent OnDeathFinished;

protected:
// ACTOR COMPONENT OVERRIDES
	virtual void OnUnregister() override;

	void ClearGameplayTags() const;

	virtual void HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);


	UFUNCTION()
	virtual void OnRep_DeathState(EDeathState OldDeathState);

protected:

	// Ability system used by this component.
	UPROPERTY()
	TObjectPtr<UBaseAbilitySystemComponent> AbilitySystemComponent;
	
	// Health set used by this component.
	UPROPERTY()
	TObjectPtr<const UHealthSet> HealthSet;

	// Replicated state used to handle dying.
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EDeathState DeathState;
};

