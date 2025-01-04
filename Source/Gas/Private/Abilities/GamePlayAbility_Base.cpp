// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/GamePlayAbility_Base.h"
#include "AbilitySystemLog.h"
#include "CameraModes/CustomCameraMode.h"
#include "Character/Components/HeroComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GamePlayAbility_Base)

UGamePlayAbility_Base::UGamePlayAbility_Base(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { ActiveCameraMode = nullptr; }

UHeroComponent* UGamePlayAbility_Base::GetHeroComponentFromActorInfo() const { return (CurrentActorInfo ? UHeroComponent::FindHeroComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr); }

void UGamePlayAbility_Base::SetCameraMode(const TSubclassOf<UCustomCameraMode>& CameraMode)
{
	if (!ensure(IsInstantiated())) // Check if the ability is instanced
	{
		ABILITY_LOG(Error, TEXT("%s:  SetCameraMode cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());
		return;
	}
	UHeroComponent* HeroComponent = GetHeroComponentFromActorInfo();
	if (!HeroComponent) return;

	HeroComponent->SetAbilityCameraMode(CameraMode, CurrentSpecHandle);
	ActiveCameraMode = CameraMode;
}

void UGamePlayAbility_Base::ClearCameraMode()
{
	if (!ensure(IsInstantiated())) // Check if the ability is instanced
	{
		ABILITY_LOG(Error, TEXT("%s:  ClearCameraMode cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());
		return;
	}
	if (!ActiveCameraMode) return;
	ActiveCameraMode = nullptr;

	if (const auto HeroComponent = GetHeroComponentFromActorInfo()) HeroComponent->ClearAbilityCameraMode(CurrentSpecHandle);
}

void UGamePlayAbility_Base::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearCameraMode();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}