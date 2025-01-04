// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPlayerCameraManager.h"

#include "Async/TaskGraphInterfaces.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "LyraCameraComponent.h"
#include "UICameraManagerComponent.h"

#include "Global/CustomCamera.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPlayerCameraManager)

class FDebugDisplayInfo;


ALyraPlayerCameraManager::ALyraPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultFOV = CAMERA_DEFAULT_FOV;
	ViewPitchMin = CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = CAMERA_DEFAULT_PITCH_MAX;

	UICameraManager = CreateDefaultSubobject<UUICameraManagerComponent>("UICameraCmp");
}

void ALyraPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	// If the UI Camera is looking at something, let it have priority.
	if (UICameraManager->NeedsToUpdateViewTarget())
	{
		Super::UpdateViewTarget(OutVT, DeltaTime);
		UICameraManager->UpdateViewTarget(OutVT, DeltaTime);
		return;
	}

	Super::UpdateViewTarget(OutVT, DeltaTime);
}

void ALyraPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& Yl, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("LyraPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, Yl, YPos);

	const APawn* Pawn = PCOwner ? PCOwner->GetPawn() : nullptr;

	if (const auto CameraComponent = ULyraCameraComponent::FindCameraComponent(Pawn)) CameraComponent->DrawDebug(Canvas);
}