#include "Ability/BaseAbilitySet.h"
#include "Ability/Abilities/BaseGameplayAbility.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "AbilitySet/GrantedHandlesData.h"
#include "AbilitySet/GameplayAbilityData.h"
#include "AbilitySet/AttributeSetData.h"
#include "AbilitySet/GameplayEffectData.h"

#include "Log/Log.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseAbilitySet)

UBaseAbilitySet::UBaseAbilitySet(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

void UBaseAbilitySet::GiveToAbilitySystem(UBaseAbilitySystemComponent* BaseAsc, FGrantedHandlesData* OutGrantedHandles,
                                          UObject* SourceObject) const
{
	check(BaseAsc);
	//must be authoritative to grant abilities, effects, and attributes
	// to give or take ability sets
	if (!BaseAsc->IsOwnerActorAuthoritative()) return;

	// Grant the attribute sets.
	GrantAttributeSets(BaseAsc, OutGrantedHandles);
	// Grant the gameplay abilities.
	GrantGameplayAbilities(BaseAsc, OutGrantedHandles, SourceObject);

	// Grant the gameplay effects.
	GrantGameplayEffects(BaseAsc, OutGrantedHandles);
}

void UBaseAbilitySet::GrantGameplayAbilities(UBaseAbilitySystemComponent* BaseASC,
                                             FGrantedHandlesData* OutGrantedHandles, UObject* SourceObject) const
{
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FGameplayAbilityData& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			ULOG_ERROR(LogGAS, "GrantedGameplayAbilities[%d] on ability set [%s] is not valid.", AbilityIndex,
			           *GetNameSafe(this));
			continue;
		}

		UBaseGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UBaseGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = BaseASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles) OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
	}
}

void UBaseAbilitySet::GrantGameplayEffects(UBaseAbilitySystemComponent* BaseAsc,
                                           FGrantedHandlesData* OutGrantedHandles) const
{
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const auto& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			ULOG_ERROR(LogGAS, "GrantedGameplayEffects[%d] on ability set [%s] is not valid", EffectIndex,
			           *GetNameSafe(this));
			continue;
		}

		const UGameplayEffect* GE = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = BaseAsc->ApplyGameplayEffectToSelf(
			GE, EffectToGrant.EffectLevel, BaseAsc->MakeEffectContext());

		if (OutGrantedHandles) OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
	}
}

void UBaseAbilitySet::GrantAttributeSets(UBaseAbilitySystemComponent* BaseASC,
                                         FGrantedHandlesData* OutGrantedHandles) const
{
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const auto& [AttributeSet] = GrantedAttributes[SetIndex];

		if (!IsValid(AttributeSet))
		{
			ULOG_ERROR(LogGAS, "GrantedAttributes[%d] on ability set [%s] is not valid", SetIndex,
			           *GetNameSafe(this));
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(BaseASC->GetOwner(), AttributeSet);
		BaseASC->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles) OutGrantedHandles->AddAttributeSet(NewSet);
	}
}
