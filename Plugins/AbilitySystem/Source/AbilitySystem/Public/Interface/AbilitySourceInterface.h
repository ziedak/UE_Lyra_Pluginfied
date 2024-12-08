#pragma once

#include "UObject/Interface.h"

#include "AbilitySourceInterface.generated.h"

class UObject;
class UPhysicalMaterial;
struct FGameplayTagContainer;

/** Base interface for anything acting as an ability calculation source */
UINTERFACE(MinimalAPI)
class UAbilitySourceInterface : public UInterface
{
	GENERATED_BODY()
};

class ABILITYSYSTEM_API IAbilitySourceInterface
{
	GENERATED_BODY()

public:
	/**
	 * Compute the multiplier for effect falloff with distance
	 *
	 * @param Distance			Distance from source to target for ability calculations (distance bullet traveled for a gun, etc...)
	 * @param SourceTags		Aggregated Tags from the source
	 * @param TargetTags		Aggregated Tags currently on the target
	 *
	 * @return Multiplier to apply to the base attribute value due to distance
	 */
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr,
	                                     const FGameplayTagContainer* TargetTags = nullptr) const = 0;

	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial,
	                                             const FGameplayTagContainer* SourceTags = nullptr,
	                                             const FGameplayTagContainer* TargetTags = nullptr) const = 0;
};
