#include "Phases/BaseGamePhaseAbility.h"
#include "Phases/BaseGamePhaseSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGamePhaseAbility)

#define LOCTEXT_NAMESPACE "UBaseGamePhaseAbility"

UBaseGamePhaseAbility::UBaseGamePhaseAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;
}
#if WITH_EDITOR
EDataValidationResult UBaseGamePhaseAbility::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!GamePhaseTag.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("GamePhaseTagInvalid", "GamePhaseTag must be set to a tag representing the current phase {0}."), FText::FromString(GetPathName())));
	}
	return Result;
}
#endif

// Called when the ability is activated
void UBaseGamePhaseAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld())
		{
			if (UBaseGamePhaseSubsystem* GamePhaseSubsystem = UWorld::GetSubsystem<UBaseGamePhaseSubsystem>(World))
			{
				GamePhaseSubsystem->OnBeginPhase(this, Handle);
			}
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBaseGamePhaseAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (const UWorld* World = ActorInfo->AbilitySystemComponent->GetWorld())
		{
			if (UBaseGamePhaseSubsystem* GamePhaseSubsystem = UWorld::GetSubsystem<UBaseGamePhaseSubsystem>(World))
			{
				GamePhaseSubsystem->OnEndPhase(this, Handle);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
#undef LOCTEXT_NAMESPACE