// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "Settings/LyraSettingsShared.h"
#include "UI/Hud/BaseHud.h"
#include "Interfaces/CameraAssistInterface.h"
#include "BasePlayerController.generated.h"


class ABasePlayerState;
class ABaseHud;
class UBaseAbilitySystemComponent;
/**
 * ABasePlayerController
 *
 *	The base player controller class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class GAS_API ABasePlayerController : public ACommonPlayerController, public ICameraAssistInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABasePlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	UFUNCTION(BlueprintCallable, Category = "Base|PlayerController")
	ABasePlayerState* GetBasePlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Base|PlayerController")
	UBaseAbilitySystemComponent* GetBaseAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Base|PlayerController")
	ABaseHud* GetBaseHUD() const;

protected:
	//~AController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;

private:
	void BroadcastOnPlayerStateChanged();
	// Called when the player state is set or cleared
	void OnPlayerStateChanged();

protected:
	//~APlayerController interface
	//virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void UpdateForceFeedback(IInputInterface* InputInterface, int32 ControllerId) override;
	virtual void UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents) override;
	virtual void PlayerTick(float DeltaTime) override;

	//~ILyraCameraAssistInterface interface
	virtual void OnCameraPenetratingTarget() override
	{
		bHideViewTargetPawnNextFrame = true;
	}

	virtual void BeginPlay() override;
	virtual void SetPlayer(UPlayer* InPlayer) override;

private:
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

	bool bHideViewTargetPawnNextFrame = false;

protected:
	void OnSettingsChanged(const ULyraSettingsShared* InSettings);
};
