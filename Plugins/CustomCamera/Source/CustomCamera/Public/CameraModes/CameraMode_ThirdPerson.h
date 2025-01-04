// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CustomCameraMode.h"

#include "Curves/CurveFloat.h"
#include "PenetrationAvoidanceFeeler.h"
#include "DrawDebugHelpers.h"
#include "CameraMode_ThirdPerson.generated.h"

class UCurveVector;

/**
 * UCameraMode_ThirdPerson
 *
 *	A basic third person camera mode.
 */
UCLASS(Abstract, Blueprintable)
class UCameraMode_ThirdPerson : public UCustomCameraMode
{
	GENERATED_BODY()

public:
	UCameraMode_ThirdPerson();

protected:
	virtual void UpdateView(float DeltaTime) override;

	void UpdateForTarget(float DeltaTime);
	void UpdatePreventPenetration(float DeltaTime);
	void PreventCameraPenetration(const class AActor& ViewTarget,
	                              const FVector& SafeLoc,
	                              FVector& CameraLoc,
	                              const float& DeltaTime,
	                              float& DistBlockedPct,
	                              bool bSingleRayOnly);

private:
	FVector CalculateRayTarget(const FVector& SafeLoc, const FVector& BaseRay, const FPenetrationAvoidanceFeeler& Feeler, const FVector& BaseRayLocalUp, const FVector& BaseRayLocalRight) const;
	void PerformSweep(const UWorld* World, const FVector& SafeLoc, const FVector& RayTarget, FCollisionShape& SphereShape, FCollisionQueryParams& SphereParams, FPenetrationAvoidanceFeeler& Feeler,
	                  const AActor& ViewTarget, float& DistBlockedPctThisFrame);
	void IgnoreHit(const FHitResult& Hit, FCollisionQueryParams& SphereParams, const AActor& ViewTarget) const;
	bool ShouldIgnoreCameraBlockingVolume(const FHitResult& Hit, const AActor& ViewTarget) const;
	float UpdateDistBlockedPctThisFrame(const FHitResult& Hit, const float DistBlockedPctThisFrame, const FVector& RayTarget, const FVector& SafeLoc);
	float UpdateDistBlockedPct(float DistBlockedPct, float DistBlockedPctThisFrame, float HardBlockedPct, float SoftBlockedPct, float DeltaTime) const;

protected:
	virtual void DrawDebug(UCanvas* Canvas) const override;

	// Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "!bUseRuntimeFloatCurves"))
	TObjectPtr<const UCurveVector> TargetOffsetCurve;

	// UE-103986: Live editing of RuntimeFloatCurves during PIE does not work (unlike curve assets).
	// Once that is resolved this will become the default and TargetOffsetCurve will be removed.
	UPROPERTY(EditDefaultsOnly, Category = "Third Person")
	bool bUseRuntimeFloatCurves = false;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetX;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetY;

	UPROPERTY(EditDefaultsOnly, Category = "Third Person", Meta = (EditCondition = "bUseRuntimeFloatCurves"))
	FRuntimeFloatCurve TargetOffsetZ;

	// Alters the speed that a crouch offset is blended in or out
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Third Person")
	float CrouchOffsetBlendMultiplier = 5.0f;

	// Penetration prevention
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	float PenetrationBlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	float PenetrationBlendOutTime = 0.15f;

	/** If true, does collision checks to keep the camera out of the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	bool bPreventPenetration = true;

	/** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	bool bDoPredictiveAvoidance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionPushOutDistance = 2.f;

	/** When the camera's distance is pushed into this percentage of its full distance due to penetration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float ReportPenetrationPercent = 0.f;

	/**
	 * These are the feeler rays that are used to find where to place the camera.
	 * Index: 0  : This is the normal feeler we use to prevent collisions.
	 * Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
	 *             were to rotate towards that direction and primitively collide the camera so that it pulls in before
	 *             impacting the occluded.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FPenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;

	UPROPERTY(Transient)
	float AimLineToDesiredPosBlockedPct = 0.f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<const AActor>> DebugActorsHitDuringCameraPenetration;

#if ENABLE_DRAW_DEBUG
	mutable float LastDrawDebugTime = -MAX_FLT;
#endif

protected:
	void SetTargetCrouchOffset(const FVector& NewTargetOffset);
	void UpdateCrouchOffset(float DeltaTime);

	FVector InitialCrouchOffset = FVector::ZeroVector;
	FVector TargetCrouchOffset = FVector::ZeroVector;
	float CrouchOffsetBlendPct = 1.0f;
	FVector CurrentCrouchOffset = FVector::ZeroVector;
};