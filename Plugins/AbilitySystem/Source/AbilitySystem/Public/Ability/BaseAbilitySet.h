#pragma once

#include "Engine/DataAsset.h"

#include "BaseAbilitySet.generated.h"

class UBaseAbilitySystemComponent;
class UObject;
struct FGameplayAbilityData;
struct FGameplayEffectData;
struct FAttributeSetData;
struct FGrantedHandlesData;
/**
 * UBaseAbilitySet
 *
 *	Non-mutable data asset used to grant gameplay abilities and gameplay effects.
 */
UCLASS(BlueprintType, Const)
class ABILITYSYSTEM_API UBaseAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UBaseAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	void GiveToAbilitySystem(UBaseAbilitySystemComponent* BaseAsc, FGrantedHandlesData* OutGrantedHandles, UObject* SourceObject = nullptr) const;

private:

	// Grants the gameplay abilities in this ability set to the specified ability system component.
	void GrantGameplayAbilities( UBaseAbilitySystemComponent* BaseASC, FGrantedHandlesData* OutGrantedHandles, UObject* SourceObject) const;

	// Grants the gameplay effects in this ability set to the specified ability system component.
	void GrantGameplayEffects(UBaseAbilitySystemComponent* BaseAsc, FGrantedHandlesData* OutGrantedHandles) const;

	// Grants the attribute sets in this ability set to the specified ability system component.
	void GrantAttributeSets(UBaseAbilitySystemComponent* BaseASC, FGrantedHandlesData* OutGrantedHandles) const;
protected:

	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (TitleProperty = Ability))
	TArray<FGameplayAbilityData> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (TitleProperty = GameplayEffect))
	TArray<FGameplayEffectData> GrantedGameplayEffects;

	// Attribute sets to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta = (TitleProperty = AttributeSet))
	TArray<FAttributeSetData> GrantedAttributes;
};
