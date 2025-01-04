// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"

#include "LyraCameraComponent.generated.h"

class UCameraModeStack;
class UCanvas;
class UCustomCameraMode;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;
template <class TClass>
class TSubclassOf;

DECLARE_DELEGATE_RetVal(TSubclassOf<UCustomCameraMode>, FCameraModeDelegate);


/**
 * ULyraCameraComponent
 *
 *	The base camera component class used by this project.
 */
UCLASS()
class CUSTOMCAMERA_API ULyraCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	ULyraCameraComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|Camera")
	static ULyraCameraComponent* FindCameraComponent(const AActor* Actor) { return Actor ? Actor->FindComponentByClass<ULyraCameraComponent>() : nullptr; }

	// Returns the target actor that the camera is looking at.
	virtual AActor* GetTargetActor() const { return GetOwner(); }

	// Delegate used to query for the best camera mode.
	FCameraModeDelegate DetermineCameraModeDelegate;

	// Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	void AddFieldOfViewOffset(const float FovOffset) { FieldOfViewOffset += FovOffset; }

	virtual void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	virtual void UpdateCameraModes();


	// Stack used to blend the camera modes.
	UPROPERTY()
	TObjectPtr<UCameraModeStack> CameraModeStack;

	// Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	float FieldOfViewOffset;
};