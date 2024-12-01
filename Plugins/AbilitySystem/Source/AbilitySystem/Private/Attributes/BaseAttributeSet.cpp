#include "Attributes/BaseAttributeSet.h"

#include "Component/BaseAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseAttributeSet)

class UWorld;

UBaseAttributeSet::UBaseAttributeSet()
{
}

UWorld* UBaseAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UBaseAbilitySystemComponent* UBaseAttributeSet::GetBaseAbilitySystemComponent() const
{
	return Cast<UBaseAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}