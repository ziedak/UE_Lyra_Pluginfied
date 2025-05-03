// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "GameplayTagContainer.h"
#include "Cheat/Interfaces/CheatServer.h"
#include "Settings/LyraSettingsShared.h"
#include "UI/Hud/BaseHud.h"
#include "Interfaces/CameraAssistInterface.h"
#include "BasePlayerController.generated.h"


class UGameplayEffect;
class UAbilitySystemComponent;
class ABasePlayerState;
class ABaseHud;
class UBaseAbilitySystemComponent;
/**
 * ABasePlayerController
 *
 *	The base player controller class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base player controller class used by this project."))
class GAS_API ABasePlayerController : public ACommonPlayerController, public ICameraAssistInterface, public ICheatServer
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
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "Base|PlayerController")
	ABaseHud* GetBaseHUD() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	void SetIsAutoRunning(const bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	bool GetIsAutoRunning() const;

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
	virtual void SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AddCheats(bool bForce) override;
	//~ILyraCameraAssistInterface interface
	virtual void OnCameraPenetratingTarget() override
	{
		bHideViewTargetPawnNextFrame = true;
	}

	//~ICheatServer interface
	// Run a cheat command on the server.
	UFUNCTION(Reliable, Server, WithValidation)
	virtual void ServerCheat(const FString& Msg) override;

	// Run a cheat command on the server for all players.
	UFUNCTION(Reliable, Server, WithValidation)
	virtual void ServerCheatAll(const FString& Msg) override;

	virtual void DamageSelfDestruct() const override;
	virtual void ToggleDynamicTagGameplayEffect(const FGameplayTag& Tag) override;
	virtual void ApplySetByCallerHeal(UAbilitySystemComponent* Asc, const FGameplayTag& Tag, const float Amount) override;
	virtual void ApplySetByCallerDamage(UAbilitySystemComponent* Asc, const FGameplayTag& Tag, const float Amount) override;

	virtual void AddDynamicTagGameplayEffect(const FGameplayTag& Tag) const override;
	virtual void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag) const override;
	virtual void CancelInputActivatedAbilities(const bool bReplicateCancelAbility) const override;

private :
	void ApplySetByCallerHealth(UAbilitySystemComponent* Asc,
	                            const FGameplayTag& Tag,
	                            const float Amount,
	                            const TSoftClassPtr<UGameplayEffect>& GameplayEffect_SetByCaller);

protected:
	virtual void BeginPlay() override;
	virtual void SetPlayer(UPlayer* InPlayer) override;

private:
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

	bool bHideViewTargetPawnNextFrame = false;

protected:
	void OnSettingsChanged(const ULyraSettingsShared* InSettings);
	void OnStartAutoRun();
	void OnEndAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnStartAutoRun"))
	void K2_OnStartAutoRun();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnEndAutoRun"))
	void K2_OnEndAutoRun();
};
