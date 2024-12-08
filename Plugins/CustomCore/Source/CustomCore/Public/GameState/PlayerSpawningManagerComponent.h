// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "PlayerSpawningManagerComponent.generated.h"

enum class EPlayerStartLocationOccupancy
{
	Empty,
	Partial,
	Full
};

class APlayerStart;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CUSTOMCORE_API UPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);
	virtual void InitializeComponent() override;

protected:
	// Utility
	APlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller,
	                                                  const TArray<APlayerStart*>& FoundStartPoints) const;

	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<APlayerStart*>& PlayerStarts) { return nullptr; }

	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName=OnFinishRestartPlayer))
	void K2_OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation);

private:
	/** We proxy these calls from ABaseGameMode, to this component so that each experience can more easily customize the respawn system they want. */
	AActor* ChoosePlayerStart(AController* Player);
	bool ControllerCanRestart(AController* Player) const;
	void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation);
	friend class ABaseGameMode;
	/** ~ABaseGameMode */

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APlayerStart>> CachedPlayerStarts;

	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	void HandleOnActorSpawned(AActor* SpawnedActor);

#if WITH_EDITOR
	APlayerStart* FindPlayFromHereStart(const AController* Player) const;
#endif                         FActorComponentTickFunction* ThisTickFunction) override;
};
