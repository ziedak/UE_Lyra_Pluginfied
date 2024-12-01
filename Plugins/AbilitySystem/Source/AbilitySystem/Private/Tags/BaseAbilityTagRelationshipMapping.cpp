#include "Tags/BaseAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseAbilityTagRelationshipMapping)

void UBaseAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutTagsToBlock,
	FGameplayTagContainer* OutTagsToCancel) const
{
	// Simple iteration for now
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (!AbilityTags.HasTag(Tags.AbilityTag))
			continue;

		if (OutTagsToBlock)
			OutTagsToBlock->AppendTags(Tags.AbilityTagsToBlock);

		if (OutTagsToCancel)
			OutTagsToCancel->AppendTags(Tags.AbilityTagsToCancel);
	}
}

void UBaseAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutActivationRequired,
	FGameplayTagContainer* OutActivationBlocked) const
{
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FAbilityTagRelationship& Tags = AbilityTagRelationships[i];
		if (!AbilityTags.HasTag(Tags.AbilityTag))
			continue;

		if (OutActivationRequired)
			OutActivationRequired->AppendTags(Tags.ActivationRequiredTags);

		if (OutActivationBlocked)
			OutActivationBlocked->AppendTags(Tags.ActivationBlockedTags);
	}
}

bool UBaseAbilityTagRelationshipMapping::IsAbilityCancelledByTag(
	const FGameplayTagContainer& AbilityTags,
	const FGameplayTag& ActionTag) const
{
	for (int32 i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const FAbilityTagRelationship& Tags = AbilityTagRelationships[i];

		if (Tags.AbilityTag == ActionTag && Tags.AbilityTagsToCancel.HasAny(AbilityTags))
			return true;
	}

	return false;
}