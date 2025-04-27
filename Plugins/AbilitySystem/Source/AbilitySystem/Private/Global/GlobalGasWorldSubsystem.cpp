#include "Global/GlobalGasWorldSubsystem.h"

#include "Global/GlobalAppliedAbilityList.h"
#include "Global/GlobalAppliedEffectList.h"
#include"Component/BaseAbilitySystemComponent.h"
#include "Log/Log.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GlobalGasWorldSubsystem)

UGlobalGasWorldSubsystem::UGlobalGasWorldSubsystem()
{
}

void UGlobalGasWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LOG_SCREEN_INFO("UGlobalGasWorldSubsystem::Initialize");
	ULOG_INFO(LogTemp, "UGlobalGasWorldSubsystem::Initialize");
};

void UGlobalGasWorldSubsystem::ApplyAbilityToAll(const TSubclassOf<UGameplayAbility> Ability)
{
	if (!Ability.Get() || AppliedAbilities.Contains(Ability))
		return;
	FGlobalAppliedAbilityList& GlobalAppliedAbilities = AppliedAbilities.Add(Ability);
	for (auto& Asc : RegisteredASCs)
	{
		GlobalAppliedAbilities.AddToAsc(Ability, Asc.Get());
	}
}

void UGlobalGasWorldSubsystem::ApplyEffectToAll(const TSubclassOf<UGameplayEffect> Effect)
{
	if (!Effect.Get() || AppliedEffects.Contains(Effect))
		return;
	FGlobalAppliedEffectList& GlobalAppliedEffects = AppliedEffects.Add(Effect);
	for (auto& Asc : RegisteredASCs)
	{
		GlobalAppliedEffects.AddToAsc(Effect, Asc.Get());
	}
}

void UGlobalGasWorldSubsystem::RemoveAbilityFromAll(const TSubclassOf<UGameplayAbility> Ability)
{
	if (!Ability.Get())
		return;
	if (FGlobalAppliedAbilityList* GlobalAppliedAbilities = AppliedAbilities.Find(Ability))
	{
		GlobalAppliedAbilities->RemoveFromAll();
		AppliedAbilities.Remove(Ability);
	}
}

void UGlobalGasWorldSubsystem::RemoveEffectFromAll(const TSubclassOf<UGameplayEffect> Effect)
{
	if (!Effect.Get())
		return;
	if (FGlobalAppliedEffectList* GlobalAppliedEffects = AppliedEffects.Find(Effect))
	{
		GlobalAppliedEffects->RemoveFromAll();
		AppliedEffects.Remove(Effect);
	}
}

void UGlobalGasWorldSubsystem::RegisterAsc(UBaseAbilitySystemComponent* Asc)
{
	check(Asc);
	RegisteredASCs.Add(Asc);

	for (auto& Pair : AppliedAbilities) { Pair.Value.AddToAsc(Pair.Key, Asc); }
	for (auto& Pair : AppliedEffects) { Pair.Value.AddToAsc(Pair.Key, Asc); }
}

void UGlobalGasWorldSubsystem::UnregisterAsc(UBaseAbilitySystemComponent* Asc)
{
	check(Asc);
	RegisteredASCs.Remove(Asc);

	for (auto& Pair : AppliedAbilities) { Pair.Value.RemoveFromAsc(Asc); }
	for (auto& Pair : AppliedEffects) { Pair.Value.RemoveFromAsc(Asc); }
}
