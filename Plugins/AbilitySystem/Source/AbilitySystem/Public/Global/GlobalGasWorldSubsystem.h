#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Global/GlobalAppliedAbilityList.h"
#include "Global/GlobalAppliedEffectList.h"
#include "GlobalGasWorldSubsystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UBaseAbilitySystemComponent;
class UObject;
struct FActiveGameplayEffectHandle;
struct FFrame;
struct FGameplayAbilitySpecHandle;
struct FGlobalAppliedAbilityList;
struct FGlobalAppliedEffectList;


/**
* The UGlobalGasWorldSubsystem provides a centralized system for managing globally-applied effects and interactions within the game world.
Here are some examples of global effects that can be managed by the UGlobalGasWorldSubsystem:
*	1.	Area-of-Effect (AoE) Damage: The UGlobalGasWorldSubsystem can handle effects that cause damage to all actors within
a certain area. For example, a fire explosion that damages all nearby actors.
*	2.	Environmental Effects: The subsystem can manage environmental effects such as weather conditions, like rain or snow, 
that affect all actors in the game world.
*	3.	Time Manipulation: The UGlobalGasWorldSubsystem can handle effects that manipulate time, such as slowing down or speeding up
time for all actors in the game world.
*	4.	Global Buffs/Debuffs: The subsystem can manage buffs or debuffs that are applied to all actors in the game world.
For example, a global buff that increases the movement speed of all actors.
*	5.	Game Rule Modifications: The UGlobalGasWorldSubsystem can handle effects that modify game rules or mechanics globally.
For instance, changing the gravity or physics behavior for all actors in the game world.
*	6.	World Events: The subsystem can manage global events that affect all actors, such as an earthquake that causes shaking 
and impacts the movement of all actors.
*	7.	Audio/Visual Effects: The UGlobalGasWorldSubsystem can handle global audio or visual effects, like changing the background
music or applying a post-processing effect to the entire game world.
 */

UCLASS()
class ABILITYSYSTEM_API UGlobalGasWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UGlobalGasWorldSubsystem();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Base")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Base")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Base")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Base")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** Register an ASC with global system and apply any active global effects/abilities. */
	void RegisterAsc(UBaseAbilitySystemComponent* Asc);

	/** Removes an ASC from the global system, along with any active global effects/abilities. */
	void UnregisterAsc(UBaseAbilitySystemComponent* Asc);

private:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UBaseAbilitySystemComponent>> RegisteredASCs;
};
