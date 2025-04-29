# UPhysicalMaterialWithTags

## Overview
`UPhysicalMaterialWithTags` is an extension of the `UPhysicalMaterial` class that includes a container for gameplay tags. These tags allow game logic to reason about the physical material in a more flexible and descriptive way.

## Features
- Adds a `FGameplayTagContainer` to physical materials for tagging and categorization.
- Enables gameplay systems to interact with physical materials based on their associated tags.

---

## Properties

### Tags
- **Type**: `FGameplayTagContainer`
- **Category**: `PhysicalProperties`
- **Description**: A container of gameplay tags that can be used by game code to reason about this physical material.

---

## Usage
1. Assign gameplay tags to a physical material in the editor or through code.
2. Use the `Tags` property in gameplay logic to query or filter physical materials based on their tags.
3. Integrate with systems like the Gameplay Ability System to trigger effects or behaviors based on the physical material's tags.

## Notes
- Ensure that the gameplay tags used are properly registered in the project's gameplay tag configuration.
- This class is useful for categorizing physical materials in a way that integrates seamlessly with gameplay systems.

## See Also
- [UPhysicalMaterial](https://docs.unrealengine.com/)
- [Gameplay Ability System](https://docs.unrealengine.com/)
- [FGameplayTagContainer](https://docs.unrealengine.com/)
