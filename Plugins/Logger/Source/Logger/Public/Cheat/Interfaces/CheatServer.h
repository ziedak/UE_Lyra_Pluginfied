// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CheatServer.generated.h"

class UAbilitySystemComponent;
struct FGameplayTag;
// This class does not need to be modified.
UINTERFACE()
class UCheatServer : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LOGGER_API ICheatServer
{
	GENERATED_BODY()

public:
	//for player controller
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const =0;
	virtual void ServerCheat(const FString& Msg) =0;
	virtual void ServerCheatAll(const FString& Msg) =0;
	virtual void DamageSelfDestruct() const =0;

	virtual void ToggleDynamicTagGameplayEffect(const FGameplayTag& Tag) =0;
	virtual void ApplySetByCallerHeal(UAbilitySystemComponent* Asc, const FGameplayTag& Tag, const float Amount) =0;
	virtual void ApplySetByCallerDamage(UAbilitySystemComponent* Asc, const FGameplayTag& Tag, const float Amount) =0;
	virtual void AddDynamicTagGameplayEffect(const FGameplayTag& Tag) const =0;
	virtual void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag) const =0;
	virtual void CancelInputActivatedAbilities(const bool bReplicateCancelAbility) const =0;
};
