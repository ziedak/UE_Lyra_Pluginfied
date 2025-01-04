#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Enums/AbilityActivationGroup.h"
#include "BaseAbilitySystemComponent.generated.h"

class AActor;
class UGameplayAbility;
class UBaseAbilityTagRelationshipMapping;
class UObject;
class UBaseGameplayAbility;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;

UCLASS()
class ABILITYSYSTEM_API UBaseAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UBaseAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

private:
	void NotifyAbilitiesOfNewPawnAvatar();
	void RegisterWithGlobalSystem();
	void InitializeAnimInstance();

public:
	using TShouldCancelAbilityFunc = TFunctionRef<bool(const UBaseGameplayAbility* BaseAbility,
	                                                   FGameplayAbilitySpecHandle Handle)>;
	void CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility);

	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	void SetAbilityInputTagPressed(const FGameplayTag& InputTag);
	void SetAbilityInputTagReleased(const FGameplayTag& InputTag);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

private:
	TSet<FGameplayAbilitySpecHandle> ProcessHeldAbilities();
	TSet<FGameplayAbilitySpecHandle> ProcessPressedAbilities();
	void ProcessReleasedAbilities();

public:
	void ClearAbilityInput();
	bool IsActivationGroupBlocked(EAbilityActivationGroup Group) const;
	void AddAbilityToActivationGroup(EAbilityActivationGroup Group, UBaseGameplayAbility* BaseAbility);
	void RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group, const UBaseGameplayAbility* Ability);
	void CancelActivationGroupAbilities(EAbilityActivationGroup Group, UBaseGameplayAbility* IgnoreAbility,
	                                    bool bReplicateCancelAbility);

	// Uses a gameplay effect to add the specified dynamic granted tag.
	void AddDynamicTagGameplayEffect(const TSoftClassPtr<UGameplayEffect>& DynamicTagGameplayEffect,
	                                 const FGameplayTag& Tag);

	// Removes all active instances of the gameplay effect that was used to add the specified dynamic granted tag.
	void RemoveDynamicTagGameplayEffect(const TSoftClassPtr<UGameplayEffect>&, const FGameplayTag& Tag);

	/** Gets the ability target data associated with the given ability handle and activation info */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle,
	                          const FGameplayAbilityActivationInfo& ActivationInfo,
	                          FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const;

	///** Sets the current tag relationship mapping, if null it will clear it out */
	FORCEINLINE void SetTagRelationshipMapping(UBaseAbilityTagRelationshipMapping* NewMapping) { TagRelationshipMapping = NewMapping; };

	/** Looks at ability tags and gathers additional required and blocking tags */
	void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags,
	                                            FGameplayTagContainer& OutActivationRequired,
	                                            FGameplayTagContainer& OutActivationBlocked) const;

protected:
	void TryActivateAbilitiesOnSpawn();

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	                                 const FGameplayTagContainer& FailureReason) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
	                                bool bWasCancelled) override;
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags,
	                                            UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
	                                            const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags,
	                                            const FGameplayTagContainer& CancelTags) override;
	virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags,
	                                              UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

	/** Notify client that an ability failed to activate
	 * @param Ability The ability that failed to activate
	 * @param FailureReason The reason the ability failed to activate
	 * @note This function is unreliable because it is called from a server RPC
	 * @note This function implements the client side of the NotifyAbilityFailedToActivate RPC
	 * @note this function implementation is ClientNotifyAbilityFailed_Implementation
	*/
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) const;

	// If set, this table is used to look up tag relationships for activate and cancel
	UPROPERTY()
	TObjectPtr<UBaseAbilityTagRelationshipMapping> TagRelationshipMapping;

	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandlesList;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandlesList;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandlesList;

	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[static_cast<uint8>(EAbilityActivationGroup::Max)];
};