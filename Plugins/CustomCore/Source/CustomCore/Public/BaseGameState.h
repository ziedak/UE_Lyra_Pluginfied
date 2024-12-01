// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularGameState.h"
#include "BaseGameState.generated.h"

class UAbilitySystemComponent;
class UExperienceManagerComponent;
struct FVerbMessage;
/**
 * 
 */
UCLASS(Config = Game)
class CUSTOMCORE_API ABaseGameState : public AModularGameStateBase, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

#pragma region Actor
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion
	void SetServerFPS(const float NewServerFPS);


#pragma region IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
#pragma endregion IAbilitySystemInterface

#pragma region AGameStateBase interface
	virtual void SeamlessTravelTransitionCheckpoint(bool bToTransitionMap) override;
#pragma endregion AGameStateBase interface

	// Send a message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Base|GameState")
	void MulticastMessageToClients_Unreliable(const FVerbMessage Message);

	// Send a message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Base|GameState")
	void MulticastMessageToClients_Reliable(const FVerbMessage Message);

	// Gets the server's FPS, replicated to clients
	float GetServerFPS() const { return ServerFPS; };

private:
	// Handles loading and managing the current gameplay experience
	UPROPERTY()
	TObjectPtr<UExperienceManagerComponent> ExperienceManagerComponent;

	// The ability system component sub-object for game-wide things (primarily gameplay cues)
	UPROPERTY(VisibleAnywhere, Category = "Base|GameState")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

protected:
	UPROPERTY(Replicated)
	float ServerFPS = 0.0f;
};
