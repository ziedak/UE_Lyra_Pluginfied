// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "Settings/LyraSettingsShared.h"
#include "UI/Hud/BaseHud.h"
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
class GAS_API ABasePlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABasePlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void SetPlayer(UPlayer* InPlayer) override;
	void OnSettingsChanged(const ULyraSettingsShared* InSettings);

	UFUNCTION(BlueprintCallable, Category = "Base|PlayerController")
	ABasePlayerState* GetBasePlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "Base|PlayerController")
	UBaseAbilitySystemComponent* GetBaseAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Base|PlayerController")
	ABaseHud* GetBaseHUD() const;

	//~AController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void InitPlayerState() override;
	void BroadcastOnPlayerStateChanged();
	void OnPlayerStateChanged();
	virtual void UpdateForceFeedback(IInputInterface* InputInterface, int32 ControllerId) override;

	//~APlayerController interface
	//virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
};
