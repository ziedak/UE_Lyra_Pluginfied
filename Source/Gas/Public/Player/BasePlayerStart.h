// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerStart.h"
#include "Interface/IPlayerSpawnInterface.h"
#include "BasePlayerStart.generated.h"


/**
 * ABasePlayerStart
 * 
 * Base player starts that can be used by a lot of modes.
 */
UCLASS(Config = Game)
class GAS_API ABasePlayerStart : public APlayerStart, public IPlayerSpawnInterface
{
	GENERATED_BODY()

public:
	ABasePlayerStart(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) {}

	const FGameplayTagContainer& GetGameplayTags() { return StartPointTags; }

	virtual EPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) override;

	/** Did this player start get claimed by a controller already? */
	bool IsClaimed() const { return ClaimingController != nullptr; };

	/** If this PlayerStart was not claimed, claim it for ClaimingController */
	virtual bool TryClaim(AController* OccupyingController) override;

protected:
	/** Check if this PlayerStart is still claimed */
	void CheckUnclaimed();

	/** The controller that claimed this PlayerStart */
	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;

	/** Interval in which we'll check if this player start is not colliding with anyone anymore */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;

	/** Tags to identify this player start */
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer StartPointTags;

	/** Handle to track expiration recurring timer */
	FTimerHandle ExpirationTimerHandle;
};
