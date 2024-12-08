#pragma once

#include "AbilitySystemComponent.h"
#include "Attributes/BaseAttributeSet.h"

#include "CombatSet.generated.h"

class UObject;
struct FFrame;


/**
 * ULyraCombatSet
 *
 *  Class that defines attributes that are necessary for applying damage or healing.
 *	Attribute examples include: damage, healing, attack power, and shield penetrations.
 */
UCLASS(BlueprintType)
class ABILITYSYSTEM_API UCombatSet : public UBaseAttributeSet
{
	GENERATED_BODY()

public:
	UCombatSet();

	ATTRIBUTE_ACCESSORS(UCombatSet, BaseDamage);
	ATTRIBUTE_ACCESSORS(UCombatSet, BaseHeal);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue) const;

	UFUNCTION()
	void OnRep_BaseHeal(const FGameplayAttributeData& OldValue) const;

private:
	// The base amount of damage to apply in the damage execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseDamage, Category = "AttributeSet|Combat",
		Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseDamage;

	// The base amount of healing to apply in the heal execution.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BaseHeal, Category = "AttributeSet|Combat",
		Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BaseHeal;
};
