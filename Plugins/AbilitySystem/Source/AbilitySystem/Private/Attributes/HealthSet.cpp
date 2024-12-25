#include "Attributes/HealthSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "MessageRuntime/GameplayMessageSubsystem.h"

#include "Component/BaseAbilitySystemComponent.h"
#include "Tags/BaseGameplayTags.h"
#include "MessageVerb/VerbMessage.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(HealthSet)

UHealthSet::UHealthSet()
	: Health(100.0f),
	  MaxHealth(100.0f),
	  bOutOfHealth(false),
	  MaxHealthBeforeAttributeChange(0.0f),
	  HealthBeforeAttributeChange(0.0f) {}

void UHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

// Attribute Set Overrides
void UHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthSet, Health, OldValue);
	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	// These events on the client should not be changing attributes

	const float CurrentHealth = GetHealth();
	const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();

	OnHealthChanged.Broadcast(
		nullptr,
		nullptr,
		nullptr,
		EstimatedMagnitude,
		OldValue.GetCurrentValue(),
		CurrentHealth);

	if (!bOutOfHealth && CurrentHealth <= 0.0f)
	{
		bOutOfHealth = true;
		OnOutOfHealth.Broadcast(
			nullptr,
			nullptr,
			nullptr,
			EstimatedMagnitude,
			OldValue.GetCurrentValue(),
			0.0f);
	}
	bOutOfHealth = CurrentHealth <= 0.0f;
}

void UHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHealthSet, MaxHealth, OldValue);

	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	const float CurrentMaxHealth = GetMaxHealth();
	const float EstimatedMagnitude = CurrentMaxHealth - OldValue.GetCurrentValue();
	OnMaxHealthChanged.Broadcast(nullptr,
	                             nullptr,
	                             nullptr,
	                             EstimatedMagnitude,
	                             OldValue.GetCurrentValue(),
	                             CurrentMaxHealth);
}

bool UHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data)) { return false; }

	// Handle modifying incoming normal damage
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		if (Data.EvaluatedData.Magnitude > 0)
		{
			const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(
				BaseGameplayTags::DAMAGE_SELF_DESTRUCT);

			// Check for damage immunity
			if (Data.Target.HasMatchingGameplayTag(BaseGameplayTags::DAMAGE_IMMUNITY) && !bIsDamageFromSelfDestruct)
			{
				Data.EvaluatedData.Magnitude = 0;
				return false;
			}
#if !UE_BUILD_SHIPPING
			// Check GodMode cheat, unlimited health is checked below
			if (Data.Target.HasMatchingGameplayTag(CheatTags::GODMODE) && !bIsDamageFromSelfDestruct)
			{
				// Do not take away any health.
				Data.EvaluatedData.Magnitude = 0.0f;
				return false;
			}
#endif // #if !UE_BUILD_SHIPPING
		}
	}
	// Save the current health
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();

	return true;
}

void UHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const float MinimumHealth = GetMinimumHealth(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetDamageAttribute()) { HandleDamage(Data, MinimumHealth); }
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute()) { HandleHealing(Data, MinimumHealth); }
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
		                             MaxHealthBeforeAttributeChange, GetMaxHealth());
	}

	if (GetHealth() != HealthBeforeAttributeChange)
	{
		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
		                          HealthBeforeAttributeChange, GetHealth());
	}

	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
		                        HealthBeforeAttributeChange, GetHealth());
	}

	bOutOfHealth = (GetHealth() <= 0.0f);
}

float UHealthSet::GetMinimumHealth(const FGameplayEffectModCallbackData& Data)
{
	const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(
		BaseGameplayTags::DAMAGE_SELF_DESTRUCT);

	float MinimumHealth = 0.0f;

#if !UE_BUILD_SHIPPING
	if (!bIsDamageFromSelfDestruct && (Data.Target.HasMatchingGameplayTag(CheatTags::GODMODE) || Data.Target.
		HasMatchingGameplayTag(CheatTags::UNLIMITED_HEALTH))) { MinimumHealth = 1.0f; }
#endif

	return MinimumHealth;
}

void UHealthSet::HandleDamage(const FGameplayEffectModCallbackData& Data, const float MinimumHealth)
{
	if (Data.EvaluatedData.Magnitude > 0.0f) { BroadcastDamageMessage(Data); }

	SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
	SetDamage(0.0f);
}

void UHealthSet::HandleHealing(const FGameplayEffectModCallbackData& Data, const float MinimumHealth)
{
	SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
	SetHealing(0.0f);
}

void UHealthSet::BroadcastDamageMessage(const FGameplayEffectModCallbackData& Data) const
{
	FVerbMessage Message;
	Message.Verb = BaseGameplayTags::DAMAGE_MESSAGE;
	Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
	Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	Message.Target = GetOwningActor();
	Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
	Message.Magnitude = Data.EvaluatedData.Magnitude;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}


// void UHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
// {
// 	//TODO:refactor this
// 	Super::PostGameplayEffectExecute(Data);
//
// 	const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(
// 		BaseGameplayTags::DAMAGE_SELF_DESTRUCT);
// 	float MinimumHealth = 0.0f;
//
// #if !UE_BUILD_SHIPPING
// 	// God-mode and unlimited health stop death unless it's a self-destruct
// 	if (!bIsDamageFromSelfDestruct &&
// 		(Data.Target.HasMatchingGameplayTag(CheatTags::GOD-MODE) || Data.Target.HasMatchingGameplayTag(
// 			CheatTags::UNLIMITED_HEALTH)))
// 	{
// 		MinimumHealth = 1.0f;
// 	}
// #endif // #if !UE_BUILD_SHIPPING
//
// 	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
// 	AActor* Instigator = EffectContext.GetOriginalInstigator();
// 	AActor* Causer = EffectContext.GetEffectCauser();
//
// 	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
// 	{
// 		// Send a standardized verb message that other systems can observe
// 		if (Data.EvaluatedData.Magnitude > 0.0f)
// 		{
// 			FVerbMessage Message;
// 			Message.Verb = BaseGameplayTags::GAS_DAMAGE_MESSAGE;
// 			Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
// 			Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
// 			Message.Target = GetOwningActor();
// 			Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
// 			//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
// 			//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...
// 			Message.Magnitude = Data.EvaluatedData.Magnitude;
//
// 			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
// 			MessageSystem.BroadcastMessage(Message.Verb, Message);
// 		}
// 		// Convert into -Health and then clamp
// 		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
// 		SetDamage(0.0f);
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
// 	{
// 		// Convert into +Health and then clamp
// 		SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
// 		SetHealing(0.0f);
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
// 	{
// 		// Clamp and fall into out of health handling below
// 		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
// 	}
// 	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
// 	{
// 		// TODO clamp current health?
//
// 		// Notify on any requested max health changes
// 		OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
// 		                             MaxHealthBeforeAttributeChange, GetMaxHealth());
// 	}
//
// 	// If health has actually changed activate callbacks
// 	if (GetHealth() != HealthBeforeAttributeChange)
// 	{
// 		OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
// 		                          HealthBeforeAttributeChange, GetHealth());
// 	}
//
// 	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
// 	{
// 		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude,
// 		                        HealthBeforeAttributeChange, GetHealth());
// 	}
//
// 	// Check health again in case an event above changed it.
// 	bOutOfHealth = (GetHealth() <= 0.0f);
// }

void UHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		//Make sure current health is not greater than new max Health
		if (GetHealth() > NewValue)
		{
			UBaseAbilitySystemComponent* Asc = GetBaseAbilitySystemComponent();
			check(Asc);
			// Override the current health to the new max health
			//TODO: Review this + find a better way to do this

			Asc->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfHealth && (GetHealth() > 0.0f)) { bOutOfHealth = false; }
}


//TODO : Review this	
void UHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}
