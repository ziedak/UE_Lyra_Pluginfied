// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BasePlayerStart.h"
#include "GameFramework/GameModeBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BasePlayerStart)

EPlayerStartLocationOccupancy ABasePlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) 
{
	UWorld* const World = GetWorld();
	if (!HasAuthority() || !World || !World->GetAuthGameMode())
		return EPlayerStartLocationOccupancy::Full;

	const auto AuthGameMode = World->GetAuthGameMode();

	const TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
	const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

	FVector ActorLocation = GetActorLocation();
	const FRotator ActorRotation = GetActorRotation();

	if (!World->EncroachingBlockingGeometry(PawnToFit,
	                                        ActorLocation,
	                                        ActorRotation,
	                                        nullptr))
	{
		return EPlayerStartLocationOccupancy::Empty;
	}

	if (World->FindTeleportSpot(PawnToFit,
	                            ActorLocation,
	                            ActorRotation))
	{
		return EPlayerStartLocationOccupancy::Partial;
	}


	return EPlayerStartLocationOccupancy::Full;
}


bool ABasePlayerStart::TryClaim(AController* OccupyingController)
{
	if (!OccupyingController || IsClaimed())
		return false;

	ClaimingController = OccupyingController;
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ExpirationTimerHandle,
		                                  FTimerDelegate::CreateUObject(this, &ABasePlayerStart::CheckUnclaimed),
		                                  ExpirationCheckInterval,
		                                  true);
	}
	return true;
}

void ABasePlayerStart::CheckUnclaimed()
{
	if (ClaimingController && ClaimingController->GetPawn() &&
		GetLocationOccupancy(ClaimingController) == EPlayerStartLocationOccupancy::Empty)
	{
		ClaimingController = nullptr;
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}
