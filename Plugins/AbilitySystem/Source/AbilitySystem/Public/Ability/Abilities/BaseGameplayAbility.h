#pragma once

#include "Abilities/GameplayAbility.h"

#include "Enums/AbilityActivationGroup.h"
#include "Enums/AbilityActivationPolicy.h"

#include "BaseGameplayAbility.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;

class AActor;
class AController;
class APlayerController;
class FText;
class IAbilitySourceInterface;
class UAnimMontage;
class UBaseAbilityCost;
class UBaseAbilitySystemComponent;
class UBaseCameraMode;
//class ULyraHeroComponent;
class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEffectSpec;
struct FGameplayEventData;

/**
 * UBaseGameplayAbility
 *
 *	The base gameplay ability class used by this project.
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class ABILITYSYSTEM_API UBaseGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class UBaseAbilitySystemComponent;

public:

	UBaseGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Base|Ability")
	UBaseAbilitySystemComponent* GetBaseAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Base|Ability")
	APlayerController* GetPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Base|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Base|Ability")
	ACharacter* GetCharacterFromActorInfo() const;

	/*UFUNCTION(BlueprintCallable, Category = "Base|Ability")
	UBaseHeroComponent* GetHeroComponentFromActorInfo() const;*/

	FORCEINLINE EAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	FORCEINLINE EAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	// Returns true if the requested activation group is a valid transition.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Base|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanChangeActivationGroup(EAbilityActivationGroup NewGroup) const;

	// Tries to change the activation group.  Returns true if it successfully changed.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Base|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ChangeActivationGroup(EAbilityActivationGroup NewGroup);

	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		ScriptOnAbilityFailedToActivate(FailedReason);
	}

	//#pragma region camera mode
	//	// Sets the ability's camera mode.
	//	UFUNCTION(BlueprintCallable, Category = "Base|Ability")
	//	void SetCameraMode(TSubclassOf<UBaseCameraMode> CameraMode);
	//
	//	// Clears the ability's camera mode.  Automatically called if needed when the ability ends.
	//	UFUNCTION(BlueprintCallable, Category = "Base|Ability")
	//	void ClearCameraMode();
	//#pragma endregion

protected:

	// Called when the ability fails to activate
	virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	// Called when the ability fails to activate
	UFUNCTION(BlueprintImplementableEvent)
	void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	virtual void OnPawnAvatarSet();

	virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const IAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

#pragma region UGameplayAbility interface

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

#pragma endregion

	// // Called when the ability is activated.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnActivateAbility")
	// void K2_OnActivateAbility(const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo, const FGameplayEventData& TriggerEventData);

	// // Called when the ability is canceled.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnCancelAbility")
	// void K2_OnCancelAbility(const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo);

	// // Called when the ability is ended.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnEndAbility")
	// void K2_OnEndAbility(const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled);

	// // Called when the ability is committed.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnCommitAbility")
	// void K2_OnCommitAbility(const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo);

	// // Called when the ability is blocked.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnBlockAbility")
	// void K2_OnBlockAbility(const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo);

	// // Called when the ability is ready to activate.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnReadyToActivate")
	// void K2_OnReadyToActivate(const FGameplayAbilityActorInfo& ActorInfo);

	// // Called when the ability is not ready to activate.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnNotReadyToActivate")
	// void K2_OnNotReadyToActivate(const FGameplayAbilityActorInfo& ActorInfo);

	// // Called when the ability is blocked by another ability.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnBlockedByAbility")
	// void K2_OnBlockedByAbility(const FGameplayAbilityActorInfo& ActorInfo);

	// // Called when the ability is blocked by another ability.
	// UFUNCTION(BlueprintImplementableEvent, Category = "Base|Ability", DisplayName = "OnBlockedByTag")
	// void K2_OnBlockedBy

#pragma region K2
	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	/** Called when this ability is removed from the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	void K2_OnAbilityRemoved();

	/** Called when the ability system is initialized with a pawn avatar. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	void K2_OnPawnAvatarSet();
#pragma endregion

protected:

	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base|Ability Activation")
	EAbilityActivationPolicy ActivationPolicy;

	// Defines the relationship between this ability activating and other abilities activating.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base|Ability Activation")
	EAbilityActivationGroup ActivationGroup;

	// Additional costs that must be paid to activate this ability
	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<UBaseAbilityCost>> AdditionalCosts;

	// Map of failure tags to simple error messages
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	// Map of failure tags to anim montages that should be played with them
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	// If true, extra information should be logged when this ability is canceled. This is temporary, used for tracking a bug.
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	bool bLogCancelation;

	// Current camera mode set by the ability.
	TSubclassOf<UBaseCameraMode> ActiveCameraMode;
};