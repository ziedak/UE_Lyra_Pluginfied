// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraModes/CameraMode_ThirdPerson.h"
#include "CameraModes/CustomCameraMode.h"
#include "Components/PrimitiveComponent.h"
#include "PenetrationAvoidanceFeeler.h"
#include "Curves/CurveVector.h"
#include "Engine/Canvas.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"

#include "Global/CustomCamera.h"

#include "Interfaces/CameraAssistInterface.h"

#include "Math/RotationMatrix.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CameraMode_ThirdPerson)

UCameraMode_ThirdPerson::UCameraMode_ThirdPerson()
{
	TargetOffsetCurve = nullptr;

	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +00.0f, 0.0f), 1.00f, 1.00f, 14.f, 0));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, -16.0f, 0.0f), 0.75f, 0.75f, 00.f, 3));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, +32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+00.0f, -32.0f, 0.0f), 0.50f, 0.50f, 00.f, 5));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(+20.0f, +00.0f, 0.0f), 1.00f, 1.00f, 00.f, 4));
	PenetrationAvoidanceFeelers.Add(FPenetrationAvoidanceFeeler(FRotator(-20.0f, +00.0f, 0.0f), 0.50f, 0.50f, 00.f, 4));
}

void UCameraMode_ThirdPerson::UpdateView(const float DeltaTime)
{
	UpdateForTarget(DeltaTime);
	UpdateCrouchOffset(DeltaTime);

	const FVector PivotLocation = GetPivotLocation() + CurrentCrouchOffset;
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;

	// Apply third person offset using pitch.
	if (bUseRuntimeFloatCurves)
	{
		FVector TargetOffset;

		TargetOffset.X = TargetOffsetX.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Y = TargetOffsetY.GetRichCurveConst()->Eval(PivotRotation.Pitch);
		TargetOffset.Z = TargetOffsetZ.GetRichCurveConst()->Eval(PivotRotation.Pitch);

		View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
		// Adjust final desired camera location to prevent any penetration
		UpdatePreventPenetration(DeltaTime);
		return;
	}

	if (TargetOffsetCurve)
	{
		const FVector TargetOffset = TargetOffsetCurve->GetVectorValue(PivotRotation.Pitch);
		View.Location = PivotLocation + PivotRotation.RotateVector(TargetOffset);
	}

	// Adjust final desired camera location to prevent any penetration
	UpdatePreventPenetration(DeltaTime);
}

void UCameraMode_ThirdPerson::UpdateForTarget(float DeltaTime)
{
	const ACharacter* TargetCharacter = Cast<ACharacter>(GetTargetActor());
	if (!TargetCharacter || !TargetCharacter->bIsCrouched)
	{
		SetTargetCrouchOffset(FVector::ZeroVector);
		return;
	}


	const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
	const float CrouchedHeightAdjustment = TargetCharacterCDO->CrouchedEyeHeight - TargetCharacterCDO->BaseEyeHeight;

	SetTargetCrouchOffset(FVector(0.f, 0.f, CrouchedHeightAdjustment));
}

void UCameraMode_ThirdPerson::DrawDebug(UCanvas* Canvas) const
{
	Super::DrawDebug(Canvas);

#if ENABLE_DRAW_DEBUG
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	for (int i = 0; i < DebugActorsHitDuringCameraPenetration.Num(); i++)
	{
		DisplayDebugManager.DrawString(
			FString::Printf(TEXT("HitActorDuringPenetration[%d]: %s")
			                , i
			                , *DebugActorsHitDuringCameraPenetration[i]->GetName()));
	}

	LastDrawDebugTime = GetWorld()->GetTimeSeconds();
#endif
}

void UCameraMode_ThirdPerson::UpdatePreventPenetration(const float DeltaTime)
{
	if (!bPreventPenetration) return;

	AActor* TargetActor = GetTargetActor();

	const APawn* TargetPawn = Cast<APawn>(TargetActor);
	AController* TargetController = TargetPawn ? TargetPawn->GetController() : nullptr;
	ICameraAssistInterface* TargetControllerAssist = Cast<ICameraAssistInterface>(TargetController);

	ICameraAssistInterface* TargetActorAssist = Cast<ICameraAssistInterface>(TargetActor);

	TOptional<AActor*> OptionalPPTarget = TargetActorAssist ? TargetActorAssist->GetCameraPreventPenetrationTarget() : TOptional<AActor*>();
	AActor* PPActor = OptionalPPTarget.IsSet() ? OptionalPPTarget.GetValue() : TargetActor;
	ICameraAssistInterface* PPActorAssist = OptionalPPTarget.IsSet() ? Cast<ICameraAssistInterface>(PPActor) : nullptr;

	const UPrimitiveComponent* PPActorRootComponent = Cast<UPrimitiveComponent>(PPActor->GetRootComponent());
	if (!PPActorRootComponent) return;
	// Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
	// Our camera is our reticule, so we want to preserve our aim and keep that as steady and smooth as possible.
	// Pick the closest point on capsule to our aim line.
	FVector ClosestPointOnLineToCapsuleCenter;
	FVector SafeLocation = PPActor->GetActorLocation();
	FMath::PointDistToLine(SafeLocation, View.Rotation.Vector(), View.Location, ClosestPointOnLineToCapsuleCenter);

	// Adjust Safe distance height to be same as aim line, but within capsule.
	const float PushInDistance = PenetrationAvoidanceFeelers[0].Extent + CollisionPushOutDistance;
	const float MaxHalfHeight = PPActor->GetSimpleCollisionHalfHeight() - PushInDistance;
	SafeLocation.Z = FMath::Clamp(ClosestPointOnLineToCapsuleCenter.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

	float DistanceSqr;
	PPActorRootComponent->GetSquaredDistanceToCollision(ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);
	// Push back inside capsule to avoid initial penetration when doing line checks.
	if (PenetrationAvoidanceFeelers.Num() > 0) SafeLocation += (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal() * PushInDistance;

	// Then aim line to desired camera position
	const bool bSingleRayPenetrationCheck = !bDoPredictiveAvoidance;
	PreventCameraPenetration(*PPActor, SafeLocation, View.Location, DeltaTime, AimLineToDesiredPosBlockedPct, bSingleRayPenetrationCheck);

	ICameraAssistInterface* AssistArray[] = {TargetControllerAssist, TargetActorAssist, PPActorAssist};

	if (AimLineToDesiredPosBlockedPct >= ReportPenetrationPercent) return;
	for (ICameraAssistInterface* Assist : AssistArray)
	{
		if (!Assist) continue;
		// camera is too close, tell the assists
		Assist->OnCameraPenetratingTarget();
	}
}

void UCameraMode_ThirdPerson::PreventCameraPenetration(const AActor& ViewTarget, const FVector& SafeLoc, FVector& CameraLoc, const float& DeltaTime, float& DistBlockedPct, bool bSingleRayOnly)
{
#if ENABLE_DRAW_DEBUG
	DebugActorsHitDuringCameraPenetration.Reset();
#endif

	float HardBlockedPct = DistBlockedPct;
	float SoftBlockedPct = DistBlockedPct;

	FVector BaseRay = CameraLoc - SafeLoc;
	FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;
	BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	float DistBlockedPctThisFrame = 1.f;

	const int32 NumRaysToShoot = bSingleRayOnly ? FMath::Min(1, PenetrationAvoidanceFeelers.Num()) : PenetrationAvoidanceFeelers.Num();
	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraPen), false, nullptr);
	SphereParams.AddIgnoredActor(&ViewTarget);

	//TODO ICameraTarget.GetIgnoredActorsForCameraPenetration();
	// if (IgnoreActorForCameraPenetration)
	// {
	// 	SphereParams.AddIgnoredActor(IgnoreActorForCameraPenetration);
	// }

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(0.f);
	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 RayIdx = 0; RayIdx < NumRaysToShoot; ++RayIdx)
	{
		FPenetrationAvoidanceFeeler& Feeler = PenetrationAvoidanceFeelers[RayIdx];
		if (Feeler.FramesUntilNextTrace > 0)
		{
			--Feeler.FramesUntilNextTrace;
			continue;
		}
		FVector RayTarget = CalculateRayTarget(SafeLoc, BaseRay, Feeler, BaseRayLocalUp, BaseRayLocalRight);
		PerformSweep(World, SafeLoc, RayTarget, SphereShape, SphereParams, Feeler, ViewTarget, DistBlockedPctThisFrame);

		if (RayIdx == 0) HardBlockedPct = DistBlockedPctThisFrame;
		else SoftBlockedPct = DistBlockedPctThisFrame;
	}

	DistBlockedPct = UpdateDistBlockedPct(DistBlockedPct, DistBlockedPctThisFrame, HardBlockedPct, SoftBlockedPct, DeltaTime);
	// UpdateCameraLocation
	if (DistBlockedPct < 1.f - ZERO_ANIMWEIGHT_THRESH) CameraLoc = SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
}


FVector UCameraMode_ThirdPerson::CalculateRayTarget(const FVector& SafeLoc, const FVector& BaseRay, const FPenetrationAvoidanceFeeler& Feeler, const FVector& BaseRayLocalUp,
                                                    const FVector& BaseRayLocalRight) const
{
	FVector RotatedRay = BaseRay.RotateAngleAxis(Feeler.AdjustmentRot.Yaw, BaseRayLocalUp);
	RotatedRay = RotatedRay.RotateAngleAxis(Feeler.AdjustmentRot.Pitch, BaseRayLocalRight);
	return SafeLoc + RotatedRay;
}

void UCameraMode_ThirdPerson::PerformSweep(const UWorld* World,
                                           const FVector& SafeLoc, const FVector& RayTarget, FCollisionShape& SphereShape, FCollisionQueryParams& SphereParams,
                                           FPenetrationAvoidanceFeeler& Feeler, const AActor& ViewTarget, float& DistBlockedPctThisFrame)
{
	SphereShape.Sphere.Radius = Feeler.Extent;
	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity, ECC_Camera, SphereShape, SphereParams);

#if ENABLE_DRAW_DEBUG
	if (World->TimeSince(LastDrawDebugTime) < 1.f)
	{
		DrawDebugSphere(World, SafeLoc, SphereShape.Sphere.Radius, 8, FColor::Red);
		DrawDebugSphere(World, bHit ? Hit.Location : RayTarget, SphereShape.Sphere.Radius, 8, FColor::Red);
		DrawDebugLine(World, SafeLoc, bHit ? Hit.Location : RayTarget, FColor::Red);
	}
#endif // ENABLE_DRAW_DEBUG

	Feeler.FramesUntilNextTrace = Feeler.TraceInterval;

	if (bHit)
	{
		IgnoreHit(Hit, SphereParams, ViewTarget);
		DistBlockedPctThisFrame = UpdateDistBlockedPctThisFrame(Hit, DistBlockedPctThisFrame, RayTarget, SafeLoc);
	}
}

void UCameraMode_ThirdPerson::IgnoreHit(const FHitResult& Hit, FCollisionQueryParams& SphereParams, const AActor& ViewTarget) const
{
	const AActor* HitActor = Hit.GetActor();
	if (!HitActor) return;

	if (HitActor->ActorHasTag(NAME_IGNORE_CAMERA_COLLISION))
	{
		SphereParams.AddIgnoredActor(HitActor);
		return;
	}

	if (!HitActor->IsA<ACameraBlockingVolume>() && ShouldIgnoreCameraBlockingVolume(Hit, ViewTarget)) SphereParams.AddIgnoredActor(HitActor);
}

bool UCameraMode_ThirdPerson::ShouldIgnoreCameraBlockingVolume(const FHitResult& Hit, const AActor& ViewTarget) const
{
	const FVector ViewTargetForwardXY = ViewTarget.GetActorForwardVector().GetSafeNormal2D();
	const FVector ViewTargetLocation = ViewTarget.GetActorLocation();
	const FVector HitOffset = Hit.Location - ViewTargetLocation;
	const FVector HitDirectionXY = HitOffset.GetSafeNormal2D();
	const float DotHitDirection = FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY);
	return DotHitDirection > 0.0f;
}

float UCameraMode_ThirdPerson::UpdateDistBlockedPctThisFrame(const FHitResult& Hit,
                                                             const float DistBlockedPctThisFrame,
                                                             const FVector& RayTarget,
                                                             const FVector& SafeLoc)
{
#if ENABLE_DRAW_DEBUG
	DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(Hit.GetActor()));
#endif

	// Recompute blocked pct taking into account push-out distance.
	const float NewBlockPct = ((Hit.Location - SafeLoc).Size() - CollisionPushOutDistance) / (RayTarget - SafeLoc).Size();
	return FMath::Min(NewBlockPct, DistBlockedPctThisFrame);
}

float UCameraMode_ThirdPerson::UpdateDistBlockedPct(const float DistBlockedPct, const float DistBlockedPctThisFrame, const float HardBlockedPct, const float SoftBlockedPct,
                                                    const float DeltaTime) const
{
	if (bResetInterpolation) return FMath::Clamp<float>(DistBlockedPctThisFrame, 0.f, 1.f);

	// if (DistBlockedPct < DistBlockedPctThisFrame)
	// {
	// 	if (PenetrationBlendOutTime > DeltaTime) { DistBlockedPct = DistBlockedPct + DeltaTime / PenetrationBlendOutTime * (DistBlockedPctThisFrame - DistBlockedPct); }
	// 	else { DistBlockedPct = DistBlockedPctThisFrame; }
	// }
	// else
	// {
	// 	if (DistBlockedPct > HardBlockedPct) { DistBlockedPct = HardBlockedPct; }
	// 	else if (DistBlockedPct > SoftBlockedPct)
	// 	{
	// 		if (PenetrationBlendInTime > DeltaTime) { DistBlockedPct = DistBlockedPct - DeltaTime / PenetrationBlendInTime * (DistBlockedPct - SoftBlockedPct); }
	// 		else { DistBlockedPct = SoftBlockedPct; }
	// 	}
	// }

	auto BlockedPct = FMath::FInterpTo(DistBlockedPct,
	                                   FMath::Min(HardBlockedPct, SoftBlockedPct),
	                                   DeltaTime,
	                                   PenetrationBlendInTime);
	if (DistBlockedPct < DistBlockedPctThisFrame)
	{
		BlockedPct = FMath::FInterpTo(DistBlockedPct,
		                              DistBlockedPctThisFrame,
		                              DeltaTime,
		                              PenetrationBlendOutTime);
	}


	return FMath::Clamp<float>(BlockedPct, 0.f, 1.f);
}


void UCameraMode_ThirdPerson::SetTargetCrouchOffset(const FVector& NewTargetOffset)
{
	CrouchOffsetBlendPct = 0.0f;
	InitialCrouchOffset = CurrentCrouchOffset;
	TargetCrouchOffset = NewTargetOffset;
}


void UCameraMode_ThirdPerson::UpdateCrouchOffset(const float DeltaTime)
{
	if (CrouchOffsetBlendPct < 1.0f)
	{
		CrouchOffsetBlendPct = FMath::Min(CrouchOffsetBlendPct + DeltaTime * CrouchOffsetBlendMultiplier, 1.0f);
		CurrentCrouchOffset = FMath::InterpEaseInOut(InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f);
		return;
	}
	CurrentCrouchOffset = TargetCrouchOffset;
	CrouchOffsetBlendPct = 1.0f;
}