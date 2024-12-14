// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemInterface.h"
#include "BasePlayerController.h"
#include "ModularPlayerState.h"
#include "BasePlayerState.generated.h"

struct FVerbMessage;
class UBaseAbilitySystemComponent;
class UGasPawnData;
class UExperienceDefinition_DA;
/** Defines the types of client connected */
UENUM()
enum class EPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};

/*
*	Base player state class used by this project.
*/
UCLASS(Config = Game)
class GAS_API ABasePlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABasePlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// UFUNCTION(BlueprintCallable, Category = "Base|PlayerState")
	// ABasePlayerController* GeBasePlayerController() const;

	UFUNCTION(BlueprintCallable, Category = "Base|PlayerState")
	UBaseAbilitySystemComponent* GetBaseAbilitySystemComponent() const { return AbilitySystemComponent; }

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UGasPawnData* InPawnData);

	//~AActor interface
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	//~End of AActor interface

	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	//~End of APlayerState interface

	static const FName NAME_BaseAbilityReady;

	void SetPlayerConnectionType(EPlayerConnectionType NewType);
	EPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }

	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "Lyra|PlayerState")
	void ClientBroadcastMessage(const FVerbMessage Message);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UGasPawnData> PawnData;

	UFUNCTION()
	void OnRep_PawnData();
	void SetReplicatedViewRotation(const FRotator& NewRot);

	// Could replace this with custom replication
	UFUNCTION()
	FRotator GetReplicatedViewRotation() const { return ReplicatedViewRotation; }


	ABasePlayerController* GetBasePlayerController() const { return Cast<ABasePlayerController>(GetOwner()); }

private:
	UFUNCTION()
	void OnExperienceLoaded(const UExperienceDefinition_DA* Experience);

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "Base|PlayerState")
	TObjectPtr<UBaseAbilitySystemComponent> AbilitySystemComponent;

	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UHealthSet> HealthSet;
	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class UCombatSet> CombatSet;

	UPROPERTY(Replicated)
	EPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY(Replicated)
	FRotator ReplicatedViewRotation;
};
