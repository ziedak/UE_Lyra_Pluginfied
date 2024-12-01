#include "Ability/Abilities/BaseGameplayAbility.h"

#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"
#include "MessageRuntime/GameplayMessageSubsystem.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "Global/BaseGameplayEffectContext.h"
#include "Interface/AbilitySourceInterface.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Ability/Cost/BaseAbilityCost.h"
#include "Ability/AbilitySimpleFailureMessage.h"
#include "Ability/AbilityMontageFailureMessage.h"
#include "Enums/AbilityActivationGroup.h"
#include "Enums/AbilityActivationPolicy.h"
#include "Tags/BaseGameplayTags.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Log/Loggger.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGameplayAbility)

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

UBaseGameplayAbility::UBaseGameplayAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = EAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EAbilityActivationGroup::Independent;

	bLogCancelation = false;
}

UBaseAbilitySystemComponent* UBaseGameplayAbility::GetBaseAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo
		        ? Cast<UBaseAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get())
		        : nullptr);
}

APlayerController* UBaseGameplayAbility::GetPlayerControllerFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<APlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
}

AController* UBaseGameplayAbility::GetControllerFromActorInfo() const
{
	if (!CurrentActorInfo)
		return nullptr;

	if (AController* PC = CurrentActorInfo->PlayerController.Get())
	{
		return PC;
	}

	// Look for a player controller or pawn in the owner chain.
	AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
	while (TestActor)
	{
		if (AController* C = Cast<AController>(TestActor))
		{
			return C;
		}

		if (APawn* Pawn = Cast<APawn>(TestActor))
		{
			return Pawn->GetController();
		}

		TestActor = TestActor->GetOwner();
	}

	return nullptr;
}

//TODO: verify this
ACharacter* UBaseGameplayAbility::GetCharacterFromActorInfo() const
{
	//return (CurrentActorInfo ? Cast<ACharacter>(CurrentActorInfo-> AvatarActor.Get()) : nullptr);
	return nullptr;
}

//UHeroComponent* UBaseGameplayAbility::GetHeroComponentFromActorInfo() const {}

void UBaseGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo,
                                                     const FGameplayAbilitySpec& Spec) const
{
	const bool bIsPredicting = (Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);

	// Try to activate if activation policy is on spawn.
	if (!ActorInfo || Spec.IsActive() || bIsPredicting || (ActivationPolicy != EAbilityActivationPolicy::OnSpawn))
		return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

	// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
	if (!ASC || !AvatarActor || AvatarActor->GetTearOff() || (AvatarActor->GetLifeSpan() > 0.0f))
		return;

	const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (
		NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
	const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (
		NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

	const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
	const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

	if (bClientShouldActivate || bServerShouldActivate)
	{
		ASC->TryActivateAbility(Spec.Handle);
	}
}

bool UBaseGameplayAbility::CanChangeActivationGroup(EAbilityActivationGroup NewGroup) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationGroup == NewGroup)
	{
		return true;
	}

	UBaseAbilitySystemComponent* BaseASC = GetBaseAbilitySystemComponentFromActorInfo();
	check(BaseASC);

	if ((ActivationGroup != EAbilityActivationGroup::Exclusive_Blocking) && BaseASC->IsActivationGroupBlocked(NewGroup))
	{
		// This ability can't change groups if it's blocked (unless it is the one doing the blocking).
		return false;
	}

	if ((NewGroup == EAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
	{
		// This ability can't become replaceable if it can't be canceled.
		return false;
	}

	return true;
}

bool UBaseGameplayAbility::ChangeActivationGroup(EAbilityActivationGroup NewGroup)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

	if (!CanChangeActivationGroup(NewGroup))
		return false;

	if (ActivationGroup != NewGroup)
	{
		UBaseAbilitySystemComponent* ASC = GetBaseAbilitySystemComponentFromActorInfo();
		check(ASC);

		ASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
		ASC->AddAbilityToActivationGroup(NewGroup, this);

		ActivationGroup = NewGroup;
	}

	return true;
}

//#pragma region camera mode
//
//void  UBaseGameplayAbility::SetCameraMode(TSubclassOf<UBaseCameraMode> CameraMode) {}
//
//void UBaseGameplayAbility::ClearCameraMode() {}
//
//#pragma endregion

void UBaseGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	//TODO: refactor this

	bool bSimpleFailureFound = false;
	for (FGameplayTag Reason : FailedReason)
	{
		// Try to find a simple failure message first before trying to play a montage
		if (!bSimpleFailureFound)
		{
			if (const FText* pUserFacingMessage = FailureTagToUserFacingMessages.Find(Reason))
			{
				FAbilitySimpleFailureMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.UserFacingReason = *pUserFacingMessage;

				UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSystem.BroadcastMessage(AbilityTags::SIMPLE_FAILURE_MESSAGE, Message);
				bSimpleFailureFound = true;
			}
		}

		// Try to find a montage to play

		if (UAnimMontage* pMontage = FailureTagToAnimMontage.FindRef(Reason))
		{
			FAbilityMontageFailureMessage Message;
			Message.PlayerController = GetActorInfo().PlayerController.Get();
			Message.FailureTags = FailedReason;
			Message.FailureMontage = pMontage;

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(AbilityTags::PLAY_MONTAGE_FAILURE_MESSAGE, Message);
		}
	}
}

void UBaseGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

void UBaseGameplayAbility::GetAbilitySource(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo,
                                            float& OutSourceLevel,
                                            const IAbilitySourceInterface*& OutAbilitySource,
                                            AActor*& OutEffectCauser) const
{
	OutSourceLevel = 0.0f;
	OutAbilitySource = nullptr;
	OutEffectCauser = nullptr;

	OutEffectCauser = ActorInfo->AvatarActor.Get();

	// If we were added by something that's an ability info source, use it
	UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

	OutAbilitySource = Cast<IAbilitySourceInterface>(SourceObject);
}

#pragma region UGameplayAbility interface

bool UBaseGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayTagContainer* SourceTags,
                                              const FGameplayTagContainer* TargetTags,
                                              FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
		return false;

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	//@TODO Possibly remove after setting up tag relationships
	const UBaseAbilitySystemComponent* Asc = CastChecked<
		UBaseAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (!Asc->IsActivationGroupBlocked(ActivationGroup))
		return true;
	// If the activation group is blocked, add the relevant tag to the OptionalRelevantTags container and return false
	if (OptionalRelevantTags)
		OptionalRelevantTags->AddTag(AbilityTags::ACTIVATE_FAIL_ACTIVATION_GROUP);

	return false;
}

void UBaseGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	// The ability can not block canceling if it's replaceable.
	if (!bCanBeCanceled && (ActivationGroup == EAbilityActivationGroup::Exclusive_Replaceable))
	{
		ULOG_ERROR(
			LogGAS,
			"SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable.",
			*GetName());
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UBaseGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UBaseGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UBaseGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBaseGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      bool bReplicateEndAbility,
                                      bool bWasCancelled)
{
	//ClearCameraMode();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UBaseGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
		return false;

	// Verify we can afford any additional costs
	for (const TObjectPtr<UBaseAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost != nullptr)
			continue;

		if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, /*inout*/ OptionalRelevantTags))
			return false;
	}

	return true;
}

void UBaseGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);
	//this a lambda function  is used to determine if the ability actually hit a target.
	// It checks if the actor is the network authority and if the ability system component has target data with hit results
	// Used to determine if the ability actually hit a target (as some costs are only spent on successful attempts)
	auto DetermineIfAbilityHitTarget = [&]()
	{
		if (ActorInfo->IsNetAuthority())
			return false;

		const UBaseAbilitySystemComponent* Asc = Cast<UBaseAbilitySystemComponent>(
			ActorInfo->AbilitySystemComponent.Get());

		if (!Asc)
			return false;

		FGameplayAbilityTargetDataHandle TargetData;
		Asc->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
		for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
		{
			if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
				return true;
		}

		return false;
	};

	// Pay any additional costs
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;
	for (const TObjectPtr<UBaseAbilityCost>& AdditionalCost : AdditionalCosts)
	{
		if (AdditionalCost == nullptr)
			continue;

		if (AdditionalCost->ShouldOnlyApplyCostOnHit())
		{
			if (!bHasDeterminedIfAbilityHitTarget)
			{
				bAbilityHitTarget = DetermineIfAbilityHitTarget();
				bHasDeterminedIfAbilityHitTarget = true;
			}

			if (!bAbilityHitTarget)
			{
				continue;
			}
		}

		AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
	}
}

// This is called when the ability is actually activated and the cost is spent
FGameplayEffectContextHandle UBaseGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle,
                                                                     const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo);
	FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);
	check(ContextHandle.IsValid());

	FBaseGameplayEffectContext* EffectContext = FBaseGameplayEffectContext::ExtractEffectContext(ContextHandle);
	check(EffectContext);

	AActor* EffectCauser = nullptr;
	const IAbilitySourceInterface* AbilitySource = nullptr;
	float SourceLevel = 0.0f;

	GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);

	const UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

	AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

	EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
	EffectContext->AddInstigator(Instigator, EffectCauser);
	EffectContext->AddSourceObject(SourceObject);

	return ContextHandle;
}

//TODO: verify this
void UBaseGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec,
                                                                FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);

	if (const FHitResult* HitResult = Spec.GetContext().GetHitResult())
	{
		if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(
			HitResult->PhysMaterial.Get()))
		{
			Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
		}
	}
} //todo: verify this
bool UBaseGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent,
                                                             const FGameplayTagContainer* SourceTags,
                                                             const FGameplayTagContainer* TargetTags,
                                                             OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// Check if any of this ability's tags are currently blocked
	if (AbilitySystemComponent.AreAbilityTagsBlocked(AbilityTags))
	{
		bBlocked = true;
	}

	const UBaseAbilitySystemComponent* ASC = Cast<UBaseAbilitySystemComponent>(&AbilitySystemComponent);

	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// Expand our ability tags to add additional required/blocked tags
	if (ASC)
	{
		ASC->GetAdditionalActivationTagRequirements(AbilityTags, AllRequiredTags, AllBlockedTags);
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags && AbilitySystemComponentTags.HasTag(StatusTags::DEATH))
			{
				// If player is dead and was rejected due to blocking tags, give that feedback
				OptionalRelevantTags->AddTag(AbilityTags::ACTIVATE_FAIL_IS_DEAD);
			}

			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}
	// Check to see if the source has any blocked or required tags for this ability to activate on it
	if (SourceTags != nullptr && (SourceBlockedTags.Num() || SourceRequiredTags.Num()))
	{
		if (SourceTags->HasAny(SourceBlockedTags))
		{
			bBlocked = true;
		}

		if (!SourceTags->HasAll(SourceRequiredTags))
		{
			bMissing = true;
		}
	}

	// Check to see if the target has any blocked or required tags for this ability to activate on it
	if (TargetTags != nullptr && (TargetBlockedTags.Num() || TargetRequiredTags.Num()))
	{
		if (TargetTags->HasAny(TargetBlockedTags))
		{
			bBlocked = true;
		}

		if (!TargetTags->HasAll(TargetRequiredTags))
		{
			bMissing = true;
		}
	}

	// If we are blocked or missing, add the relevant tags to the OptionalRelevantTags container
	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}

	// If we are missing required tags, add the relevant tags to the OptionalRelevantTags container
	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}
#pragma endregion
