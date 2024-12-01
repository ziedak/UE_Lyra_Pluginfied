#include "Character/SharedRepMovement.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SharedRepMovement)

FSharedRepMovement::FSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

// This function is crucial for synchronizing character movement data across the network in Unreal Engine. 
bool FSharedRepMovement::FillForCharacter(const ACharacter* Character)
{
	const USceneComponent* PawnRootComponent = Character->GetRootComponent();
	if (!PawnRootComponent)
		return false;

	const UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

	RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
	RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
	RepMovement.LinearVelocity = CharacterMovement->Velocity;
	RepMovementMode = CharacterMovement->PackNetworkMovementMode();
	bProxyIsJumpForceApplied = Character->bProxyIsJumpForceApplied || (Character->JumpForceTimeRemaining > 0.0f);
	bIsCrouched = Character->bIsCrouched;

	// Timestamp is sent as zero if unused
	RepTimeStamp = 0.f;
	if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->
		bNetworkAlwaysReplicateTransformUpdateTimestamp)
		RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();


	return true;
}

bool FSharedRepMovement::Equals(const FSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location)
		return false;

	if (RepMovement.Rotation != Other.RepMovement.Rotation)
		return false;

	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
		return false;

	if (RepMovementMode != Other.RepMovementMode)
		return false;

	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
		return false;

	if (bIsCrouched != Other.bIsCrouched)
		return false;

	return true;
}
// ensures that the character's movement state is correctly transmitted over the network
bool FSharedRepMovement::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Ar, Map, bOutSuccess);
	Ar << RepMovementMode;
	Ar << bProxyIsJumpForceApplied;
	Ar << bIsCrouched;

	// Timestamp, if non-zero.
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Ar.SerializeBits(&bHasTimeStamp, 1);

	if (bHasTimeStamp)
	{
		Ar << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

return true;
}
