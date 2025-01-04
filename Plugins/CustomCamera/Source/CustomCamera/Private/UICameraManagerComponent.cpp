// Copyright Epic Games, Inc. All Rights Reserved.

#include "UICameraManagerComponent.h"

#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "LyraPlayerCameraManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UICameraManagerComponent)

class AActor;
class FDebugDisplayInfo;

UUICameraManagerComponent* UUICameraManagerComponent::GetComponent(const APlayerController* PC)
{
	if (!PC) return nullptr;
	const auto CameraManager = Cast<ALyraPlayerCameraManager>(PC->PlayerCameraManager);
	return CameraManager ? CameraManager->GetUICameraComponent() : nullptr;
}

UUICameraManagerComponent::UUICameraManagerComponent()
{
	bWantsInitializeComponent = true;

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		// Register "showdebug" hook.
		if (!IsRunningDedicatedServer()) AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
	}
}

void UUICameraManagerComponent::InitializeComponent() { Super::InitializeComponent(); }

void UUICameraManagerComponent::SetViewTarget(AActor* InViewTarget, const FViewTargetTransitionParams TransitionParams)
{
	TGuardValue UpdatingViewTargetGuard(bUpdatingViewTarget, true);

	ViewTarget = InViewTarget;
	CastChecked<ALyraPlayerCameraManager>(GetOwner())->SetViewTarget(ViewTarget, TransitionParams);
}