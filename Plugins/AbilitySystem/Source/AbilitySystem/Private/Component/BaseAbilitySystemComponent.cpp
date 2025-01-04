#include"Component/BaseAbilitySystemComponent.h"

#include "Global/GlobalGasWorldSubsystem.h"
#include "AssetManager/BaseAssetManager.h"
#include "Ability/Abilities/BaseGameplayAbility.h"
// #include "Data/GasGameData.h"
#include "Enums/AbilityActivationGroup.h"
#include "Enums/AbilityActivationPolicy.h"
#include "Tags/BaseGameplayTags.h"
#include "Tags/BaseAbilityTagRelationshipMapping.h"
#include "Log/Log.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseAbilitySystemComponent)

UBaseAbilitySystemComponent::UBaseAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ClearAbilityInput();
	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UBaseAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister with the global system when the component is destroyed.
	if (UGlobalGasWorldSubsystem* GlobalGasWorldSubsystem = UWorld::GetSubsystem<UGlobalGasWorldSubsystem>(GetWorld())) GlobalGasWorldSubsystem->UnregisterAsc(this);

	Super::EndPlay(EndPlayReason);
}

/**
 * Initializes the ability actor info for the component.
 *
 * @param InOwnerActor The owner actor of the component.
 * @param InAvatarActor The avatar actor associated with the component.
 */
void UBaseAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (!bHasNewPawnAvatar) return;

	NotifyAbilitiesOfNewPawnAvatar();
	RegisterWithGlobalSystem();
	InitializeAnimInstance();
	TryActivateAbilitiesOnSpawn();
}

void UBaseAbilitySystemComponent::NotifyAbilitiesOfNewPawnAvatar()
{
	// Notify all abilities that a new pawn avatar has been set
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		ensureMsgf(AbilitySpec.Ability && AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced,
		           TEXT("InitAbilityActorInfo: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			if (const auto BaseGameplayAbility = Cast<UBaseGameplayAbility>(AbilityInstance))
			{
				// Ability instances may be missing for replays
				BaseGameplayAbility->OnPawnAvatarSet();
			}
		}
	}
}

void UBaseAbilitySystemComponent::RegisterWithGlobalSystem()
{
	// Register with the global system once we actually have a pawn avatar.
	// We wait until this time since some globally-applied effects may require an avatar.
	if (UGlobalGasWorldSubsystem* GlobalGasWorldSubsystem = UWorld::GetSubsystem<
		UGlobalGasWorldSubsystem>(GetWorld()))
		GlobalGasWorldSubsystem->RegisterAsc(this);
}

//@TODO: Implement this
void UBaseAbilitySystemComponent::InitializeAnimInstance()
{
	/*if (UBaseAnimInstance* BaseAnimInst = Cast<UBaseAnimInstance>(ActorInfo->GetAnimInstance()))
	{
		BaseAnimInst->InitializeWithAbilitySystem(this);
	}*/
}

void UBaseAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UBaseGameplayAbility* BaseAbilityCDO = Cast<UBaseGameplayAbility>(AbilitySpec.Ability)) BaseAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
	}
}

void UBaseAbilitySystemComponent::CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, const bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive()) continue;

		const UBaseGameplayAbility* BaseAbilityCDO = Cast<UBaseGameplayAbility>(AbilitySpec.Ability);
		if (!BaseAbilityCDO)
		{
			UE_LOG(LogGAS, Error, TEXT("CancelAbilitiesByFunc: Non-BaseGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced,
		           TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		// Cancel all the spawned instances.
		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			UBaseGameplayAbility* BaseAbilityInstance = CastChecked<UBaseGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(BaseAbilityInstance, AbilitySpec.Handle))
			{
				if (BaseAbilityInstance->CanBeCanceled())
				{
					BaseAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), BaseAbilityInstance->GetCurrentActivationInfo(),
					                                   bReplicateCancelAbility);
				}
				else { UE_LOG(LogGAS, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *BaseAbilityInstance->GetName()); }
			}
		}
	}
}

// This function cancels all abilities that are currently active.
void UBaseAbilitySystemComponent::CancelInputActivatedAbilities(const bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UBaseGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle){
		const EAbilityActivationPolicy ActivationPolicy = Ability->GetActivationPolicy();
		return ((ActivationPolicy == EAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy ==
			EAbilityActivationPolicy::WhileInputActive));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

// This function sets the ability input tag that was pressed.
void UBaseAbilitySystemComponent::SetAbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
		{
			InputPressedSpecHandlesList.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandlesList.AddUnique(AbilitySpec.Handle);
		}
	}
}

// This function sets the ability input tag as released.
void UBaseAbilitySystemComponent::SetAbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
		{
			InputReleasedSpecHandlesList.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandlesList.Remove(AbilitySpec.Handle);
		}
	}
}

// This function processes the ability input.
void UBaseAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// If the ability system is blocked, clear the ability input and return.
	if (HasMatchingGameplayTag(BaseGameplayTags::ABILITY_INPUT_BLOCKED))
	{
		ClearAbilityInput();
		return;
	}
	// Process all abilities that are held, pressed, and released.
	TSet<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Append(ProcessHeldAbilities());
	AbilitiesToActivate.Append(ProcessPressedAbilities());

	// Try to activate all the abilities that are from presses and holds.
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate) { TryActivateAbility(AbilitySpecHandle); }

	ProcessReleasedAbilities();
	// Clear the cached ability handles.
	InputPressedSpecHandlesList.Reset();
	InputReleasedSpecHandlesList.Reset();
}

// This function processes all abilities that are held.
TSet<FGameplayAbilitySpecHandle> UBaseAbilitySystemComponent::ProcessHeldAbilities()
{
	TSet<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	// Process all abilities that activate when the input is held.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandlesList)
	{
		// Check if the ability should be activated.
		const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec)
		{
			LOG_WARNING(LogGAS, "ProcessHeldAbilitiesProcessHeldAbilities: AbilitySpec is null. Handle ");
			continue;
		}
		if (!AbilitySpec->IsActive())
		{
			const UBaseGameplayAbility* BaseAbilityCDO = Cast<UBaseGameplayAbility>(AbilitySpec->Ability);
			if (BaseAbilityCDO && BaseAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::WhileInputActive) AbilitiesToActivate.Add(AbilitySpec->Handle);
		}
	}

	return AbilitiesToActivate;
}

// This function processes all abilities that had their input pressed this frame.
TSet<FGameplayAbilitySpecHandle> UBaseAbilitySystemComponent::ProcessPressedAbilities()
{
	TSet<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	// Process all abilities that had their input pressed this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandlesList)
	{
		// Set the input pressed flag for the ability.
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec)
		{
			LOG_WARNING(LogGAS, "ProcessPressedAbilities: AbilitySpec is null. Handle ");
			continue;
		}
		AbilitySpec->InputPressed = true;

		if (AbilitySpec->IsActive()) AbilitySpecInputPressed(*AbilitySpec);
		else
		{
			// If the ability is not active, check if it should be activated.
			const UBaseGameplayAbility* BaseAbilityCDO = Cast<UBaseGameplayAbility>(AbilitySpec->Ability);

			if (BaseAbilityCDO && BaseAbilityCDO->GetActivationPolicy() == EAbilityActivationPolicy::OnInputTriggered) AbilitiesToActivate.Add(AbilitySpec->Handle);
		}
	}

	return AbilitiesToActivate;
}

// This function processes all abilities that had their input released this frame.
void UBaseAbilitySystemComponent::ProcessReleasedAbilities()
{
	// Process all abilities that had their input released this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandlesList)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec) continue;

		AbilitySpec->InputPressed = false;

		if (AbilitySpec->IsActive()) AbilitySpecInputReleased(*AbilitySpec);
	}
}

// This function clears the ability input.
void UBaseAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandlesList.Reset();
	InputReleasedSpecHandlesList.Reset();
	InputHeldSpecHandlesList.Reset();
}

// This function checks if the specified activation group is blocked.
bool UBaseAbilitySystemComponent::IsActivationGroupBlocked(EAbilityActivationGroup Group) const
{
	switch (Group)
	{
	case EAbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
		return false;
	case EAbilityActivationGroup::Exclusive_Replaceable:
	case EAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		return (ActivationGroupCounts[static_cast<uint8>(EAbilityActivationGroup::Exclusive_Blocking)] > 0);
	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), static_cast<uint8>(Group));
		return false;
	}
}

// This function adds the ability to the specified activation group.
void UBaseAbilitySystemComponent::AddAbilityToActivationGroup(EAbilityActivationGroup Group,
                                                              UBaseGameplayAbility* BaseAbility)
{
	check(BaseAbility);
	check(ActivationGroupCounts[static_cast<uint8>(Group)] < INT32_MAX);

	// Increment the count for this group.
	ActivationGroupCounts[static_cast<uint8>(Group)]++;

	// If this ability is exclusive, cancel all other abilities in the same group.
	if (Group == EAbilityActivationGroup::Exclusive_Replaceable || Group == EAbilityActivationGroup::Exclusive_Blocking)
	{
		constexpr bool bReplicateCancelAbility = false;
		CancelActivationGroupAbilities(Group, BaseAbility, bReplicateCancelAbility);
	}

	// Check for multiple exclusive abilities running.
	const int32 ExclusiveCount = ActivationGroupCounts[static_cast<uint8>(
			EAbilityActivationGroup::Exclusive_Replaceable)] +
		ActivationGroupCounts[static_cast<uint8>(EAbilityActivationGroup::Exclusive_Blocking)];
	if (ExclusiveCount > 1) { LOG_ERROR(LogGAS, "AddAbilityToActivationGroup: Multiple exclusive abilities are running."); }
}

// This function removes the ability from the specified activation group.
void UBaseAbilitySystemComponent::RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group,
                                                                   const UBaseGameplayAbility* Ability)
{
	check(Ability);
	const auto GroupUnit = static_cast<uint8>(Group);
	check(ActivationGroupCounts[GroupUnit] > 0);

	ActivationGroupCounts[GroupUnit]--;
}

// This function cancels all abilities in the specified activation group.
void UBaseAbilitySystemComponent::CancelActivationGroupAbilities(EAbilityActivationGroup Group,
                                                                 UBaseGameplayAbility* IgnoreAbility,
                                                                 const bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const UBaseGameplayAbility* Ability,
	                                                     FGameplayAbilitySpecHandle Handle){
		return ((Ability->GetActivationGroup() == Group) && (Ability != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

// This function adds the dynamic tag gameplay effect with the specified tag.
void UBaseAbilitySystemComponent::AddDynamicTagGameplayEffect(
	const TSoftClassPtr<UGameplayEffect>& DynamicTagGameplayEffect, const FGameplayTag& Tag)
{
	// const TSubclassOf<UGameplayEffect> DynamicTagGE = UBaseAssetManager::GetSubclass(
	// 	UGasGameData::Get().DynamicTagGameplayEffect);
	const TSubclassOf<UGameplayEffect> DynamicTagGE = UBaseAssetManager::GetSubclass(DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		// LOG_WARNING(LogGAS, "AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s].",
		//                    *UGasGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		LOG_WARNING(LogGAS, "AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s].",
		            *DynamicTagGameplayEffect.GetAssetName());
		return;
	}

	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

	if (!Spec)
	{
		LOG_WARNING(LogGAS, "AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s].",
		            *GetNameSafe(DynamicTagGE));
		return;
	}

	Spec->DynamicGrantedTags.AddTag(Tag);

	ApplyGameplayEffectSpecToSelf(*Spec);
}

// This function removes the dynamic tag gameplay effect with the specified tag.
void UBaseAbilitySystemComponent::RemoveDynamicTagGameplayEffect(
	const TSoftClassPtr<UGameplayEffect>& DynamicTagGameplayEffect, const FGameplayTag& Tag)
{
	// Find the DynamicTagGameplayEffect.
	// const TSubclassOf<UGameplayEffect> DynamicTagGE = UBaseAssetManager::GetSubclass(
	// 	UGasGameData::Get().DynamicTagGameplayEffect);
	const TSubclassOf<UGameplayEffect> DynamicTagGE = UBaseAssetManager::GetSubclass(DynamicTagGameplayEffect);
	if (!DynamicTagGE)
	{
		// LOG_WARNING(LogGAS, "RemoveDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s].",
		//                    *UGasGameData::Get().DynamicTagGameplayEffect.GetAssetName());
		LOG_WARNING(LogGAS, "RemoveDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s].",
		            *DynamicTagGameplayEffect.GetAssetName());
		return;
	}

	// Create a query that matches any active GE with the specified tag.
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	Query.EffectDefinition = DynamicTagGE;
	// Remove any active GEs that match the query.
	RemoveActiveEffects(Query);
}

//
void UBaseAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle,
                                                       const FGameplayAbilityActivationInfo& ActivationInfo,
                                                       FGameplayAbilityTargetDataHandle& OutTargetDataHandle) const
{
	// Find the cached target data for this ability handle.
	const auto Key = FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle,
	                                                            ActivationInfo.GetActivationPredictionKey());
	if (const TSharedPtr<FAbilityReplicatedDataCache> RepData = AbilityTargetDataMap.Find(Key); RepData.IsValid()) OutTargetDataHandle = RepData->TargetData;
}

// This function is called when an ability's target data is set.
void UBaseAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags,
                                                                         FGameplayTagContainer& OutActivationRequired,
                                                                         FGameplayTagContainer& OutActivationBlocked)
const
{
	if (!TagRelationshipMapping) return;
	// Get the required and blocked activation tags from the tag relationship mapping.
	TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired,
	                                                            &OutActivationBlocked);
}

// This function is called when an ability's input is pressed.
void UBaseAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (!Spec.IsActive())
	{
		LOG_WARNING(LogGAS, "AbilityInputTagPressed: AbilitySpec is not active. Ability [%s] ",
		            *Spec.Ability->GetName());
		return;
	}
	// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
	const FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
	InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
}

// This function is called when an ability's input is released.
void UBaseAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (!Spec.IsActive())
	{
		LOG_WARNING(LogGAS, "AbilityInputTagReleased: AbilitySpec is not active. Ability [%s] ",
		            *Spec.Ability->GetName());
	}

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.

	// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
	const FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
	InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
}

// This function is called when an ability is activated.
void UBaseAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                                         UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UBaseGameplayAbility* BaseAbility = Cast<UBaseGameplayAbility>(Ability)) AddAbilityToActivationGroup(BaseAbility->GetActivationGroup(), BaseAbility);
}

// This function is called when an ability fails to activate.
void UBaseAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle,
                                                      UGameplayAbility* Ability,
                                                      const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	const APawn* Avatar = Cast<APawn>(GetAvatarActor());
	if (!Avatar)
	{
		LOG_WARNING(LogGAS, "NotifyAbilityFailed: Avatar is not a pawn. Ability [%s] ", *Ability->GetName());
		return;
	}

	if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
	{
		ClientNotifyAbilityFailed(Ability, FailureReason);
		LOG_WARNING(LogGAS, "NotifyAbilityFailed: Ability [%s] failed. Replicating to client. ",
		            *Ability->GetName());
		return;
	}

	HandleAbilityFailed(Ability, FailureReason);
}

// This function is called when an ability ends.
void UBaseAbilitySystemComponent::NotifyAbilityEnded(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability,
                                                     const bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);
	if (const UBaseGameplayAbility* BaseAbility = Cast<UBaseGameplayAbility>(Ability)) RemoveAbilityFromActivationGroup(BaseAbility->GetActivationGroup(), BaseAbility);
}

// This function is called when an ability is blocked or canceled.
void UBaseAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags,
                                                                 UGameplayAbility* RequestingAbility,
                                                                 const bool bEnableBlockTags,
                                                                 const FGameplayTagContainer& BlockTags,
                                                                 const bool bExecuteCancelTags,
                                                                 const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifierBlockTags;
	FGameplayTagContainer ModifierCancelTags;

	if (TagRelationshipMapping) TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifierBlockTags, &ModifierCancelTags);

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags,
	                                      bExecuteCancelTags, CancelTags);
	//@TODO: Apply any special logic like blocking input or movement
}

// This function is called when an ability's ability can be canceled state changes.
void UBaseAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags,
                                                                   UGameplayAbility* RequestingAbility,
                                                                   const bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: Apply any special logic like blocking input or movement
}

/**
 *  This function is called when an ability fails to activate.
 * @param Ability
 * @param FailureReason
 */
void UBaseAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(
	const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) { HandleAbilityFailed(Ability, FailureReason); }

/**
 *  This function is called when an ability fails to activate.
 * @param Ability
 * @param FailureReason
 */
void UBaseAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability,
                                                      const FGameplayTagContainer& FailureReason) const
{
	if (const UBaseGameplayAbility* BaseAbility = Cast<const UBaseGameplayAbility>(Ability)) BaseAbility->OnAbilityFailedToActivate(FailureReason);
}