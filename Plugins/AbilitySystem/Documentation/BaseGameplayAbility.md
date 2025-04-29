# UBaseGameplayAbility

## Overview
`UBaseGameplayAbility` is the base class for all gameplay abilities in the project. It extends `UGameplayAbility` and provides additional functionality such as activation policies, activation groups, and custom costs.

## Properties
- **ActivationPolicy**: Defines how this ability is meant to activate (e.g., manual or automatic).
- **ActivationGroup**: Defines the relationship between this ability activating and other abilities activating.
- **AdditionalCosts**: A list of additional costs that must be paid to activate this ability.
- **FailureTagToUserFacingMessages**: A map of failure tags to user-facing error messages.
- **FailureTagToAnimMontage**: A map of failure tags to animation montages that should be played.
- **bLogCancelation**: If true, logs additional information when this ability is canceled.

## Methods
- **GetBaseAbilitySystemComponentFromActorInfo**: Retrieves the ability system component from the actor info.
- **GetPlayerControllerFromActorInfo**: Retrieves the player controller from the actor info.
- **GetControllerFromActorInfo**: Retrieves the controller from the actor info.
- **GetCharacterFromActorInfo**: Retrieves the character from the actor info.
- **CanChangeActivationGroup**: Checks if the requested activation group is a valid transition.
- **ChangeActivationGroup**: Tries to change the activation group and returns true if successful.
- **TryActivateAbilityOnSpawn**: Attempts to activate the ability when the actor spawns.
- **OnAbilityFailedToActivate**: Called when the ability fails to activate.

## Events
- **K2_OnAbilityAdded**: Called when this ability is granted to the ability system component.
- **K2_OnAbilityRemoved**: Called when this ability is removed from the ability system component.
- **K2_OnPawnAvatarSet**: Called when the ability system is initialized with a pawn avatar.

## Overrides
- **CanActivateAbility**: Checks if the ability can be activated.
- **SetCanBeCanceled**: Sets whether the ability can be canceled.
- **OnGiveAbility**: Called when the ability is granted.
- **OnRemoveAbility**: Called when the ability is removed.
- **ActivateAbility**: Activates the ability.
- **CheckCost**: Checks if the cost of the ability can be paid.
- **ApplyCost**: Applies the cost of the ability.
- **MakeEffectContext**: Creates a gameplay effect context.
- **ApplyAbilityTagsToGameplayEffectSpec**: Applies ability tags to a gameplay effect spec.
- **DoesAbilitySatisfyTagRequirements**: Checks if the ability satisfies tag requirements.

## Usage
This class is used as the base for all gameplay abilities in the project. It provides a framework for defining activation policies, costs, and failure handling.
