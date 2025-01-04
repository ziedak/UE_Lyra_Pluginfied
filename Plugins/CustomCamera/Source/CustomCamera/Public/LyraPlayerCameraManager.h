// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/PlayerCameraManager.h"

#include "LyraPlayerCameraManager.generated.h"

class FDebugDisplayInfo;
class UCanvas;
class UObject;
class UUICameraManagerComponent;

/**
 * ALyraPlayerCameraManager
 *
 *	The base player camera manager class used by this project.
 */
UCLASS(notplaceable, MinimalAPI)
class ALyraPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ALyraPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	UUICameraManagerComponent* GetUICameraComponent() const { return UICameraManager; };

protected:
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& Yl, float& YPos) override;

private:
	/** The UI Camera Component, controls the camera when UI is doing something important that gameplay doesn't get priority over. */
	UPROPERTY(Transient)
	TObjectPtr<UUICameraManagerComponent> UICameraManager;
};