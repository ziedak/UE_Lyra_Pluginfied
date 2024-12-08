#pragma once
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
//#include "Core/Interface/AbilitySourceInterface.h"
#include "BaseGameplayEffectContext.generated.h"

class AActor;
class FArchive;
class UObject;
class UPhysicalMaterial;
class IAbilitySourceInterface;

/**
 * Data structure that stores an instigator and related data, such as positions and targets
 * Games can subclass this structure and add game-specific information
 * It is passed throughout effect execution so it is a great place to track transient information about an execution
 */

USTRUCT()
struct ABILITYSYSTEM_API FBaseGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FBaseGameplayEffectContext()
		: FGameplayEffectContext()
	{
	}

	FBaseGameplayEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: FGameplayEffectContext(InInstigator, InEffectCauser)
	{
	}

	virtual FGameplayAbilityTargetDataHandle GetTargetData()
	{
		return TargetData;
	}

	virtual void AddTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
	{
		TargetData.Append(TargetDataHandle);
	}


	/** Returns the wrapped FLyraGameplayEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static FBaseGameplayEffectContext* ExtractEffectContext(struct FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(const IAbilitySourceInterface* InObject, float InSourceLevel);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	const IAbilitySourceInterface* GetAbilitySource() const;

	/** Returns the physical material from the hit result if there is one */
	const UPhysicalMaterial* GetPhysicalMaterial() const;

#pragma region FGameplayEffectContext Override

	/**
	* Functions that subclasses of FGameplayEffectContext need to override
	*/

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual FBaseGameplayEffectContext* Duplicate() const override
	{
		FBaseGameplayEffectContext* NewContext = new FBaseGameplayEffectContext();
		*NewContext = *this;
		NewContext->AddActors(Actors);
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

#pragma endregion

protected:
	FGameplayAbilityTargetDataHandle TargetData;

	/** Ability Source object (should implement IBaseAbilitySourceInterface). NOT replicated currently */
	UPROPERTY()
	TWeakObjectPtr<const UObject> AbilitySourceObject;
};

template <>
struct TStructOpsTypeTraits<FBaseGameplayEffectContext> : TStructOpsTypeTraitsBase2<FBaseGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true // Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
