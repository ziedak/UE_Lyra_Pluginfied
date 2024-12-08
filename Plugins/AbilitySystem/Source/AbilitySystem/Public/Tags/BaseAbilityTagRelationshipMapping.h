#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "BaseAbilityTagRelationshipMapping.generated.h"

class UObject;

/* Struct that defines the relationship between different ability tags
 * This is used to define how different abilities interact with each other based on their tags
 * For example,
	Ability has the tag "Fire",
		it might block abilities with the tag "Ice" and cancel abilities with the tag "Water"
		And it might require the tag "Mana" to be activated, and block the tag "Silence" from activating it
 *

*/
USTRUCT()
struct FAbilityTagRelationship
{
	GENERATED_BODY()

	/** The tag that this container relationship is about. Single tag, but abilities can have multiple of these */
	UPROPERTY(EditAnywhere, Category = Ability, meta = (Categories = "Gameplay.Action"))
	FGameplayTag AbilityTag;

	/** The other ability tags that will be blocked by any ability using this tag */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToBlock;

	/** The other ability tags that will be canceled by any ability using this tag */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToCancel;

	/** If an ability has the tag, this is implicitly added to the activation required tags of the ability */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationRequiredTags;

	/** If an ability has the tag, this is implicitly added to the activation blocked tags of the ability */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationBlockedTags;
};

/** Mapping of how ability tags block or cancel other abilities  */
UCLASS()
class UBaseAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

	/** The list of relationships between different gameplay tags (which ones block or cancel others) */
	UPROPERTY(EditAnywhere, Category = Ability, meta = (TitleProperty = "AbilityTag"))
	TArray<FAbilityTagRelationship> AbilityTagRelationships;

public:
	/** Given a set of ability tags, parse the tag relationship and fill out tags to block and cancel 
	 * @param AbilityTags The tags of the ability that is being checked
	 * @param OutTagsToBlock The tags that will be blocked by the ability
	 * @param OutTagsToCancel The tags that will be canceled by the ability
	*/
	void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock,
	                                    FGameplayTagContainer* OutTagsToCancel) const;

	/** Given a set of ability tags, add additional required and blocking tags 
	 * @param AbilityTags The tags of the ability that is being checked
	 * @param OutActivationRequired The tags that are required for activation
	 * @param OutActivationBlocked The tags that are blocked for activation
	*/
	void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags,
	                                         FGameplayTagContainer* OutActivationRequired,
	                                         FGameplayTagContainer* OutActivationBlocked) const;

	/** Returns true if the specified ability tags are canceled by the passed in action tag 
	 * @param AbilityTags The tags of the ability that is being checked
	 * @param ActionTag The tag of the action that is being checked
	 * @return True if the ability is canceled by the action tag
	*/
	bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;
};
