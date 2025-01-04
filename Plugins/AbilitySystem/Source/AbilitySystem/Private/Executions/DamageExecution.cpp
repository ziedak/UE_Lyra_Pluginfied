#include "Executions/DamageExecution.h"
#include "Attributes/CombatSet.h"
#include "Global/BaseGameplayEffectContext.h"
#include "Interface/AbilitySourceInterface.h"

#include "Log/Log.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DamageExecution)

struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

	FDamageStatics()
	{
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(UCombatSet::GetBaseDamageAttribute(),
		                                                          EGameplayEffectAttributeCaptureSource::Source, true);
	};
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics DmgStatics;
	return DmgStatics;
}

UDamageExecution::UDamageExecution() { RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef); }

/*
1.	The code is wrapped in an #if WITH_SERVER_CODE preprocessor directive, which means it will only be compiled if the WITH_SERVER_CODE flag is defined.
	This is typically used to include server - specific code that should not be executed on the client.
2.	The function takes two parameters : ExecutionParams of type FGameplayEffectCustomExecutionParameters and OutExecutionOutput of type
	FGameplayEffectCustomExecutionOutput.These parameters provide information about the execution context and allow the function to modify the execution output.
3.	The function starts by extracting the necessary information from the ExecutionParams and Spec objects.
	It retrieves the TypedContext, which is an instance of FBaseGameplayEffectContext that contains contextual information about the effect.
	It also retrieves the SourceTags and TargetTags, which are containers of gameplay tags associated with the effect.
4.	The function then attempts to calculate the value of the base damage attribute by calling AttemptCalculateCapturedAttributeMagnitude on the ExecutionParams.
	If the calculation fails, the base damage will be set to 0.
	==> This is important to ensure that the execution can continue even if the attribute is not set.
5.	the function initializes variables for the hit actor, impact location, impact normal, start trace, and end trace.
	These variables will be used to determine the hit actor, surface, zone, and distance.
6.	The code checks if there is a hit result available.If there is, it retrieves the hit actor from the hit result and updates the impact location,
	impact normal, start trace, and end trace variables accordingly.
7.	If there is no hit result or the hit result does not return an actor to hit, the code checks if there is a target ability system component(TargetASC).
	If there is, it sets the hit actor to the avatar actor of the target ability system component.
	==> This is important for effects that are not targeted at an actor but are applied to the world, such as point-blank AOE effects or weapon fire.
8.	The code then initializes a variable DamageInteractionAllowedMultiplier to 0. This variable will be used to apply rules for team damage, self damage, etc.
	However, this part of the code is currently commented out.
9.	The code calculates the distance between the effect causer and the hit actor.This distance is important for determining the falloff of damage based on distance.
10.	The code applies ability source modifiers to the base damage.Ability source modifiers can include things like critical strike damage or damage bonuses
	from abilities.It also applies damage falloff based on distance and damage interaction rules(which are currently commented out).
11.	The code calculates the final damage by multiplying the base damage by the physical material attenuation, distance attenuation,
	and damage interaction allowed multiplier.It then clamps the damage to a minimum of 0.
12.	If the damage done is greater than 0, the code adds a damage modifier to the output.This modifier is of type FGameplayModifierEvaluatedData and
	represents the negative damage that will be applied to the target.The modifier is added to the OutExecutionOutput object.

	Overall, this code calculates the final damage based on various factors such as base damage, distance, physical material attenuation, and damage interaction rules.
	It then sets the execution output with the calculated damage modifier.
*/

void UDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// This is the implementation of the damage execution. It is called when the damage effect is executed.
	// It is responsible for calculating the final damage and setting the execution output.

#if WITH_SERVER_CODE

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FBaseGameplayEffectContext* TypedContext = FBaseGameplayEffectContext::ExtractEffectContext(Spec.GetContext());
	check(TypedContext);

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float BaseDamage = 0.f;
	// Attempt to capture the base damage attribute. If it fails, the base damage will be 0.
	// This is important for cases where the attribute is not set, but the execution still needs to continue.
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef,
	                                                           EvaluationParameters,
	                                                           BaseDamage);

	const AActor* EffectCauser = TypedContext->GetEffectCauser();
	const FHitResult* HitActorResult = TypedContext->GetHitResult();

	AActor* HitActor = nullptr;
	FVector ImpactLocation = FVector::ZeroVector;
	FVector ImpactNormal = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;
	FVector EndTrace = FVector::ZeroVector;

	// Calculation of hit actor, surface, zone, and distance all rely on whether the calculation has a hit result or not.
	// Effects just being added directly w/o having been targeted will always come in without a hit result, which must default
	// to some fallback information.
	// If there is a hit result, we can use that to determine the hit actor, surface, zone, and distance.

	if (HitActorResult)
	{
		const FHitResult& CurHitResult = *HitActorResult;
		HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor == nullptr)
		{
			ImpactLocation = CurHitResult.ImpactPoint;
			ImpactNormal = CurHitResult.ImpactNormal;
			StartTrace = CurHitResult.TraceStart;
			EndTrace = CurHitResult.TraceEnd;
		}
	}

	// Handle case of no hit result or hit result not actually returning an actor to hit.
	// This is the case for effects that are not targeted at an actor, but are just applied to the world.
	// In this case, we need to default to the effect causer as the hit actor.
	// If the effect causer is not set, we default to the owning actor of the ability system component.
	// This is important for things like point-blank AOE effects.
	// This is important for things like weapon fire, where the effect causer is not set, but the owning actor is the weapon.

	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!HitActor)
	{
		HitActor = TargetASC ? TargetASC->GetAvatarActor_Direct() : nullptr;
		if (HitActor) ImpactLocation = HitActor->GetActorLocation();
	}

	// Apply rules for team damage/self damage/etc...
	constexpr float DamageInteractionAllowedMultiplier = 0.0f;
	/*
	if (HitActor)
	{
		ULyraTeamSubsystem* TeamSubsystem = HitActor->GetWorld()->GetSubsystem<ULyraTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			DamageInteractionAllowedMultiplier = TeamSubsystem->CanCauseDamage(EffectCauser, HitActor) ? 1.0 : 0.0;
		}
	}*/

	//Determinate the distance between the effect causer and the hit actor.
	//This is important for determining the falloff of damage based on distance.
	double Distance = WORLD_MAX;
	if (TypedContext->HasOrigin()) Distance = FVector::Dist(TypedContext->GetOrigin(), ImpactLocation);
	else if (EffectCauser) Distance = FVector::Dist(EffectCauser->GetActorLocation(), ImpactLocation);
	else
	{
		UE_LOG(LogGAS, Error,
		       TEXT(
			       "Damage Calculation cannot deduce a source location for damage coming from %s; Falling back to WORLD_MAX dist!"
		       ), *GetPathNameSafe(Spec.Def));
	}

	// Apply ability source modifiers
	// This is where we apply the ability source modifiers to the base damage.
	// This is important for things like critical strike damage, or damage bonuses from abilities.
	// This is also where we apply the damage falloff based on distance.
	// This is also where we apply the damage interaction rules (team damage, self damage, etc...).
	float PhysicalMaterialAttenuation = 1.0f;
	float DistanceAttenuation = 1.0f;

	// Apply the physical material attenuation and distance attenuation to the base damage.
	if (const IAbilitySourceInterface* AbilitySource = TypedContext->GetAbilitySource())
	{
		if (const UPhysicalMaterial* PhysMat = TypedContext->GetPhysicalMaterial())
		{
			PhysicalMaterialAttenuation = AbilitySource->
				GetPhysicalMaterialAttenuation(PhysMat, SourceTags, TargetTags);
		}
		DistanceAttenuation = AbilitySource->GetDistanceAttenuation(Distance);
	}
	DistanceAttenuation = FMath::Max(DistanceAttenuation, 0.0f);

	//Clamping is done When damage is converted to _health
	const float DamageDone = FMath::Max(
		BaseDamage * PhysicalMaterialAttenuation * DistanceAttenuation * DamageInteractionAllowedMultiplier, 0.0f);
	if (DamageDone > 0)
	{
		//Apply a damage modifier , this gets turned int -health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UCombatSet::GetBaseDamageAttribute(),
		                                                                    EGameplayModOp::Additive,
		                                                                    -DamageDone));
	}
#endif
}