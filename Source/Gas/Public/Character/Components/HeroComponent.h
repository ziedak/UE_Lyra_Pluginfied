// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "HeroComponent.generated.h"

class ULyraInputConfig_DA;
// class UBaseCameraMode;
struct FInputMappingContextAndPriority;
// @TODO :implement function that uses CameraMode


/**
 * Component that sets up input and camera handling for player controlled pawns (or bots that simulate players).
 * This depends on a PawnExtensionComponent to coordinate initialization.
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class GAS_API UHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHeroComponent(const FObjectInitializer& ObjectInitializer);

	/** Returns the hero component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "Lyra|Hero")
	static UHeroComponent* FindHeroComponent(const AActor* Actor)
	{
		return (Actor ? Actor->FindComponentByClass<UHeroComponent>() : nullptr);
	}

	/** Overrides the camera from an active gameplay ability */
	//void SetAbilityCameraMode(TSubclassOf<UBaseCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Clears the camera override if it is set */
	//void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

	/** Adds mode-specific input config */
	void AddAdditionalInputConfig(const ULyraInputConfig_DA* InputConfig);

	/** Removes a mode-specific input config if it has been added */
	void RemoveAdditionalInputConfig(const ULyraInputConfig_DA* InputConfig) const;

	/** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added */
	FORCEINLINE bool IsReadyToBindInputs() const { return bReadyToBindInputs; };

	/** The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
	static const FName NAME_BIND_INPUTS_NOW;

	/** The name of this component-implemented feature */
	static const FName NAME_ACTOR_FEATURE_NAME;

	//~ Begin IGameFrameworkInitStateInterface interface
#pragma region IGameFrameworkInitStateInterface
	virtual FName GetFeatureName() const override { return NAME_ACTOR_FEATURE_NAME; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	                                FGameplayTag DesiredState) const override;

	bool CanTransitionToSpawned(const APawn* Pawn) const { return Pawn != nullptr; };
	bool CanTransitionToDataAvailable(const APawn* Pawn) const;
	bool CanTransitionToDataInitialized(const UGameFrameworkComponentManager* Manager, APawn* Pawn) const;
	bool CanTransitionToGameplayReady() const;

	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	                                   FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
#pragma endregion

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	void RegisterInputMappings(UEnhancedInputLocalPlayerSubsystem* Subsystem);
	void BindInputActions(UInputComponent* PlayerInputComponent, const UEnhancedInputLocalPlayerSubsystem* Subsystem);

	void Input_AbilityInputTagPressed(const FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(const FGameplayTag InputTag);

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_AutoRun(const FInputActionValue& InputActionValue);

	//TSubclassOf<UBaseCameraMode> DetermineCameraMode() const;

	UPROPERTY(EditAnywhere)
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;

	/** Camera mode set by an ability. */
	// UPROPERTY()
	// TSubclassOf<UBaseCameraMode> AbilityCameraMode;

	/** Spec handle for the last ability to set a camera mode. */
	// FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	/** True when player input bindings have been applied, will never be true for non - players */
	bool bReadyToBindInputs;
};
