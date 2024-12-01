#include "Global/BaseGameplayEffectContext.h"

#include "Interface/AbilitySourceInterface.h"
//Engine
#include "Abilities/GameplayAbilityTargetTypes.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGameplayEffectContext)

class FArchive;

FBaseGameplayEffectContext* FBaseGameplayEffectContext::ExtractEffectContext(FGameplayEffectContextHandle Handle)
{
	FGameplayEffectContext* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(FBaseGameplayEffectContext::StaticStruct()))
	{
		return static_cast<FBaseGameplayEffectContext*>(BaseEffectContext);
	}

	return nullptr;
}

void FBaseGameplayEffectContext::SetAbilitySource(const IAbilitySourceInterface* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<const UObject>(InObject));
}

const IAbilitySourceInterface* FBaseGameplayEffectContext::GetAbilitySource() const
{
	return Cast<IAbilitySourceInterface>(AbilitySourceObject.Get());
}

const UPhysicalMaterial* FBaseGameplayEffectContext::GetPhysicalMaterial() const
{
	if (const FHitResult* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}

bool FBaseGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return Super::NetSerialize(Ar, Map, bOutSuccess) && TargetData.NetSerialize(Ar, Map, bOutSuccess);
}
#if UE_WITH_IRIS
namespace UE::Net
{
	// Forward to FGameplayEffectContextNetSerializer
	// Note: If FFBaseGameplayEffectContext::NetSerialize() is modified, a custom Net Serializer must be implemented as the current fallback will no longer be sufficient.
	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(FBaseGameplayEffectContext, FGameplayEffectContextNetSerializer);
}
#endif