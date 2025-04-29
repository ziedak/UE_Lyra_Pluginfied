// Copyright Epic Games, Inc. All Rights Reserved.


#include "Character/Components/BaseCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Tags/BaseGameplayTags.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseCharacterMovementComponent)


namespace Character
{
	static auto GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("Character.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};

void UBaseCharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
		return;
	}

	Super::SimulateMovement(DeltaTime);
}

bool UBaseCharacterMovementComponent::CanAttemptJump() const
{
	// Same as UBaseCharacterMovementComponent's implementation but without the crouch check
	return IsJumpAllowed() &&
		(IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}


const FCharacterGroundInfo& UBaseCharacterMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
		return CachedGroundInfo;

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - Character::GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = Character::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
			CachedGroundInfo.GroundDistance = 0.0f;
		else if (HitResult.bBlockingHit)
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}

void UBaseCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}

FRotator UBaseCharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	const auto Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	return (Asc && Asc->HasMatchingGameplayTag(BaseGameplayTags::MOVEMENT_STOPPED)) ? FRotator(0, 0, 0) : Super::GetDeltaRotation(DeltaTime);
}

float UBaseCharacterMovementComponent::GetMaxSpeed() const
{
	const auto Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	return (Asc && Asc->HasMatchingGameplayTag(BaseGameplayTags::MOVEMENT_STOPPED)) ? 0 : Super::GetMaxSpeed();
}
