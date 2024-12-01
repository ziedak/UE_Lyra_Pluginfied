#include "Executions/HealExecution.h"
#include "Attributes/HealthSet.h"
#include "Attributes/CombatSet.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(HealExecution)

struct FHealStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseHealDef;

	FHealStatics()
	{
		BaseHealDef = FGameplayEffectAttributeCaptureDefinition(UCombatSet::GetBaseHealAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true);
	}
};

static FHealStatics& HealStatics()
{
	static FHealStatics Statics;
	return Statics;
}

UHealExecution::UHealExecution()
{
	RelevantAttributesToCapture.Add(HealStatics().BaseHealDef);
}

void UHealExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float BaseHeal = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().BaseHealDef,
		EvaluateParameters,
		BaseHeal);

	if (const float HealingDone = FMath::Max(BaseHeal, 0.f))
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthSet::GetHealingAttribute(),
			EGameplayModOp::Additive,
			HealingDone));
	}
#endif // #if WITH_SERVER_CODE
}