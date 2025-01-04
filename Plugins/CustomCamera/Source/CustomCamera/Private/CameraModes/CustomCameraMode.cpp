// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraModes/CustomCameraMode.h"

#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/Character.h"
#include "LyraCameraComponent.h"
#include "LyraPlayerCameraManager.h"

#include "CustomCameraTypes/CameraModeBlendFunction.h"

#include "Global/CustomCamera.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CustomCameraMode)


UCustomCameraMode::UCustomCameraMode()
{
	FieldOfView = CAMERA_DEFAULT_FOV;
	ViewPitchMin = CAMERA_DEFAULT_PITCH_MIN;
	ViewPitchMax = CAMERA_DEFAULT_PITCH_MAX;

	BlendTime = 0.5f;
	BlendFunction = ECameraModeBlendFunction::EaseOut;
	BlendExponent = 4.0f;
	BlendAlpha = 1.0f;
	BlendWeight = 1.0f;

	bResetInterpolation = false;
}

ULyraCameraComponent* UCustomCameraMode::GetCameraComponent() const { return CastChecked<ULyraCameraComponent>(GetOuter()); }

UWorld* UCustomCameraMode::GetWorld() const { return HasAnyFlags(RF_ClassDefaultObject) ? nullptr : GetOuter()->GetWorld(); }

AActor* UCustomCameraMode::GetTargetActor() const
{
	const auto CameraComponent = GetCameraComponent();

	return CameraComponent->GetTargetActor();
}

FVector UCustomCameraMode::GetPivotLocation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	const APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!TargetPawn) return TargetActor->GetActorLocation();

	const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn);
	// Height adjustments for characters to account for crouching.
	if (!TargetCharacter) return TargetPawn->GetPawnViewLocation();

	const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
	check(TargetCharacterCDO);

	const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
	check(CapsuleComp);

	const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
	check(CapsuleCompCDO);

	const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
	const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	const float HeightAdjustment = DefaultHalfHeight - ActualHalfHeight + TargetCharacterCDO->BaseEyeHeight;

	return TargetCharacter->GetActorLocation() + FVector::UpVector * HeightAdjustment;
}

FRotator UCustomCameraMode::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	if (const APawn* TargetPawn = Cast<APawn>(TargetActor)) return TargetPawn->GetViewRotation();

	return TargetActor->GetActorRotation();
}

void UCustomCameraMode::UpdateCameraMode(const float DeltaTime)
{
	UpdateView(DeltaTime);
	UpdateBlending(DeltaTime);
}

void UCustomCameraMode::UpdateView(float DeltaTime)
{
	const FVector PivotLocation = GetPivotLocation();
	FRotator PivotRotation = GetPivotRotation();

	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}

void UCustomCameraMode::SetBlendWeight(const float Weight)
{
	BlendWeight = FMath::Clamp(Weight, 0.0f, 1.0f);

	// Since we're setting the blend weight directly, we need to calculate the blend alpha to account for the blend function.
	const float InvExponent = BlendExponent > 0.0f ? 1.0f / BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case ECameraModeBlendFunction::Linear: BlendAlpha = BlendWeight;
		break;
	case ECameraModeBlendFunction::EaseIn: BlendAlpha = FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		break;
	case ECameraModeBlendFunction::EaseOut: BlendAlpha = FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;
	case ECameraModeBlendFunction::EaseInOut: BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		break;
	default:
		checkf(false, TEXT("SetBlendWeight: Invalid BlendFunction [%d]\n"), static_cast<uint8>(BlendFunction));
		break;
	}
}

void UCustomCameraMode::UpdateBlending(const float DeltaTime)
{
	if (BlendTime > 0.0f)
	{
		BlendAlpha += DeltaTime / BlendTime;
		BlendAlpha = FMath::Min(BlendAlpha, 1.0f);
	}
	else BlendAlpha = 1.0f;

	const float Exponent = BlendExponent > 0.0f ? BlendExponent : 1.0f;

	switch (BlendFunction)
	{
	case ECameraModeBlendFunction::Linear: BlendWeight = BlendAlpha;
		break;
	case ECameraModeBlendFunction::EaseIn: BlendWeight = FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		break;
	case ECameraModeBlendFunction::EaseOut: BlendWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;
	case ECameraModeBlendFunction::EaseInOut: BlendWeight = FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		break;
	default:
		checkf(false, TEXT("UpdateBlending: Invalid BlendFunction [%d]\n"), static_cast<uint8>(BlendFunction));
		break;
	}
}

void UCustomCameraMode::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("      CustomCameraMode: %s (%f)"), *GetName(), BlendWeight));
}