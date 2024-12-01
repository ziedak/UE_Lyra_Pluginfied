#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "HealExecution.generated.h"

/**
 * UHealExecution
 *
 *	Execution used by gameplay effects to apply healing to the health attributes.
 */
UCLASS()
class ABILITYSYSTEM_API UHealExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UHealExecution();
protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};