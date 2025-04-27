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

ULyraCameraComponent* UCustomCameraMode::GetCameraComponent() const
{
	return CastChecked<ULyraCameraComponent>(GetOuter());
}

UWorld* UCustomCameraMode::GetWorld() const
{
	return !HasAnyFlags(RF_ClassDefaultObject) ? GetOuter()->GetWorld() : nullptr;
}

AActor* UCustomCameraMode::GetTargetActor() const
{
	const auto CameraComponent = GetCameraComponent();
	if (!CameraComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("CameraComponent is null"));
		return nullptr;
	}
	return CameraComponent->GetTargetActor();
}

/** Get the pivot location for the camera mode.
 *  This is the location that the camera will orbit around.
 *  The default implementation uses the target actor's location.
 *  Override this function to provide a custom pivot location.
 */
FVector UCustomCameraMode::GetPivotLocation() const
{
	// Get the target actor from the camera component.
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	UE_LOG(LogTemp, Verbose, TEXT("TargetActor: %s"), *TargetActor->GetName());

	const APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!TargetPawn)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Target is not a pawn. Using actor location."));
		return TargetActor->GetActorLocation();
	}

	const ACharacter* TargetCharacter = Cast<ACharacter>(TargetPawn);
	if (!TargetCharacter)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Target is a pawn but not a character. Using pawn view location."));
		return TargetPawn->GetPawnViewLocation();
	}

	const ACharacter* TargetCharacterCDO = TargetCharacter->GetClass()->GetDefaultObject<ACharacter>();
	check(TargetCharacterCDO);

	const UCapsuleComponent* CapsuleComp = TargetCharacter->GetCapsuleComponent();
	if (!CapsuleComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetCharacter has no capsule component. Using actor location."));
		return TargetCharacter->GetActorLocation();
	}

	const UCapsuleComponent* CapsuleCompCDO = TargetCharacterCDO->GetCapsuleComponent();
	if (!CapsuleCompCDO)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetCharacterCDO has no capsule component. Using actor location."));
		return TargetCharacter->GetActorLocation();
	}

	const float DefaultHalfHeight = CapsuleCompCDO->GetUnscaledCapsuleHalfHeight();
	const float ActualHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	const float HeightAdjustment = DefaultHalfHeight - ActualHalfHeight + TargetCharacterCDO->BaseEyeHeight;

	UE_LOG(LogTemp, Verbose, TEXT("HeightAdjustment: %f"), HeightAdjustment);

	return TargetCharacter->GetActorLocation() + FVector::UpVector * HeightAdjustment;
}

FRotator UCustomCameraMode::GetPivotRotation() const
{
	const AActor* TargetActor = GetTargetActor();
	check(TargetActor);

	const APawn* TargetPawn = Cast<APawn>(TargetActor);
	check(TargetPawn);

	return TargetPawn ? TargetPawn->GetViewRotation() : TargetActor->GetActorRotation();
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
	const float InvExponent = BlendExponent > 0.0f ? 1.0f / BlendExponent : 1.0f;

	BlendAlpha = [this, InvExponent]()
	{
		switch (BlendFunction)
		{
		case ECameraModeBlendFunction::Linear:
			return BlendWeight;
		case ECameraModeBlendFunction::EaseIn:
			return FMath::InterpEaseIn(0.0f, 1.0f, BlendWeight, InvExponent);
		case ECameraModeBlendFunction::EaseOut:
			return FMath::InterpEaseOut(0.0f, 1.0f, BlendWeight, InvExponent);
		case ECameraModeBlendFunction::EaseInOut:
			return FMath::InterpEaseInOut(0.0f, 1.0f, BlendWeight, InvExponent);
		default: checkf(false, TEXT("Invalid BlendFunction [%d]"), static_cast<uint8>(BlendFunction));
			return 0.0f;
		}
	}();
}

void UCustomCameraMode::UpdateBlending(const float DeltaTime)
{
	BlendAlpha = BlendTime > 0.0f ? FMath::Min(BlendAlpha + (DeltaTime / BlendTime), 1.0f) : 1.0f;
	const float Exponent = BlendExponent > 0.0f ? BlendExponent : 1.0f;

	BlendWeight = [this, Exponent]()
	{
		switch (BlendFunction)
		{
		case ECameraModeBlendFunction::Linear:
			return BlendAlpha;
		case ECameraModeBlendFunction::EaseIn:
			return FMath::InterpEaseIn(0.0f, 1.0f, BlendAlpha, Exponent);
		case ECameraModeBlendFunction::EaseOut:
			return FMath::InterpEaseOut(0.0f, 1.0f, BlendAlpha, Exponent);
		case ECameraModeBlendFunction::EaseInOut:
			return FMath::InterpEaseInOut(0.0f, 1.0f, BlendAlpha, Exponent);
		default: checkf(false, TEXT("Invalid BlendFunction [%d]"), static_cast<uint8>(BlendFunction));
			return 0.0f;
		}
	}();
}


void UCustomCameraMode::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("      CustomCameraMode: %s (%f)"), *GetName(), BlendWeight));
}
