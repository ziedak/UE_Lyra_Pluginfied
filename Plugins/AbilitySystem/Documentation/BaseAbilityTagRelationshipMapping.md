
# UBaseAbilityTagRelationshipMapping

## Overview
`UBaseAbilityTagRelationshipMapping` defines relationships between gameplay tags, such as which tags block or cancel others.

## Properties
- **AbilityTagRelationships**: A list of relationships between gameplay tags.

## Methods
- **GetAbilityTagsToBlockAndCancel**: Determines tags to block and cancel based on input tags.
- **GetRequiredAndBlockedActivationTags**: Adds required and blocked tags for activation.
- **IsAbilityCancelledByTag**: Checks if an ability is canceled by a specific tag.
