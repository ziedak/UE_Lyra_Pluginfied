# BaseGameplayTags

## Overview
`BaseGameplayTags` defines a collection of gameplay tags used throughout the project. These tags are categorized into namespaces such as `StateTags`, `AbilityTags`, `InputTags`, and more.

## Namespaces and Tags
- **StateTags**: Tags related to character states (e.g., `DEAD`, `KNOCKED_DOWN`).
- **AbilityTags**: Tags related to abilities (e.g., `ABILITY`, `ACTIVATE_FAIL_COOLDOWN`).
- **InputTags**: Tags for input actions (e.g., `MOVE`, `JUMP`).
- **GameplayCueTags**: Tags for gameplay cues (e.g., `CHARACTER_DAMAGE_TAKEN`).
- **GameplayEffectTags**: Tags for gameplay effects (e.g., `HEAL`, `DAMAGETRAIT_BASIC`).
- **SetByCallerTags**: Tags for dynamic values (e.g., `DAMAGE`, `HEAL`).
- **CheatTags**: Tags for cheat-related functionality (e.g., `GODMODE`).
- **StatusTags**: Tags for character statuses (e.g., `CROUCHING`, `AUTORUNNING`).
- **MovementTags**: Tags for movement modes (e.g., `MODE_WALKING`, `MODE_FLYING`).

## Methods
- None.

## Events
- None.

## Overrides
- None.

## Usage
1. Use these tags to define and manage gameplay states, abilities, and interactions.
2. Ensure that all tags are registered in the project's gameplay tag configuration.

## Notes
- Ensure that all gameplay tags are registered in the project's gameplay tag configuration to avoid runtime issues.
- Use a consistent naming convention for tags to maintain clarity and organization.

## See Also
- [UBaseGameplayAbility](UBaseGameplayAbility.md)
- [UBaseAbilitySystemComponent](UBaseAbilitySystemComponent.md)
- [UBaseGamePhaseSubsystem](UBaseGamePhaseSubsystem.md)
