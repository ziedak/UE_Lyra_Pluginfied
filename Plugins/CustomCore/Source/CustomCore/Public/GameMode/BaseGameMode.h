// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Experience/DataAsset/UserFacingExperienceDefinition_DA.h>

#include "ModularGameMode.h"
#include "BaseGameMode.generated.h"

enum class ECommonUserOnlineContext : uint8;
class UCommonSession_HostSessionRequest;
class UExperienceDefinition_DA;
enum class ECommonUserPrivilege : uint8;
enum class ECommonSessionOnlineMode : uint8;
class UCommonUserInfo;
class UGasPawnData;
/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non-seamless travel
 *
 * This is called after the player has finished initialization
 */


/** 
 * ALyraGameMode
 *
 *	The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class CUSTOMCORE_API ABaseGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	ABaseGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()): Super(ObjectInitializer) {}

	// UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
	//  UGasPawnData* GetPawnDataForController(const AController* InController) const;

#pragma region AGameModeBase interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	// virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	// // virtual APawn*
	// SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	// We never want to use the start spot, always use the spawn management component.
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override { return false; };
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void InitGameState() override;
	// Do nothing, we'll wait until PostLogin when we try to spawn the player for real.
	// Doing anything right now is no good, systems like team assignment haven't even occurred yet.
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override { return false; };
	virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
#pragma  endregion

	void ServerRestartBot(AController* Controller) const;
	// Restart (respawn) the specified player or bot next frame
	// - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// Agnostic version of PlayerCanRestart that can be used for both player bots and players
	virtual bool ControllerCanRestart(AController* Controller);

	// Delegate called on player initialization, described above
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLyraGameModePlayerInitialized, AGameModeBase* /*GameMode*/,
	                                     AController* /*NewPlayer*/);
	FOnLyraGameModePlayerInitialized OnGameModePlayerInitialized;

protected:
	void OnExperienceLoaded(const UExperienceDefinition_DA* CurrentExperience);
	bool IsExperienceLoaded() const;

	void OnMatchAssignmentGiven(const FPrimaryAssetId& ExperienceId, const FString& ExperienceIdSource) const;
	void HandleMatchAssignmentIfNotExpectingOne();

private:
	FPrimaryAssetId GetExperienceFromOptions(FString& ExperienceIdSource) const;
	FPrimaryAssetId GetExperienceFromEditor(FString& ExperienceIdSource) const;
	FPrimaryAssetId GetExperienceFromCommandLine(FString& ExperienceIdSource) const;
	FPrimaryAssetId GetExperienceFromWorldSettings(FString& ExperienceIdSource) const;
	FPrimaryAssetId GetDefaultExperience(FString& ExperienceIdSource);
	bool ValidateExperienceAssetData(const FPrimaryAssetId& ExperienceId) const;

protected:
	bool TryDedicatedServerLogin();
	void HostDedicatedServerMatch(ECommonSessionOnlineMode OnlineMode) const;
	UUserFacingExperienceDefinition_DA* LoadUserExperience() const;
	FPrimaryAssetId GetUserExperienceIdFromCommandLine(const FPrimaryAssetType& UserExperienceType) const;
	UUserFacingExperienceDefinition_DA* FindUserExperience(const FPrimaryAssetType& UserExperienceType,
	                                                       const FPrimaryAssetId& UserExperienceId) const;
	UCommonSession_HostSessionRequest* CreateHostSessionRequest(
		const UUserFacingExperienceDefinition_DA* UserExperience,
		ECommonSessionOnlineMode OnlineMode) const;

	void HostGameSession(UCommonSession_HostSessionRequest* HostRequest) const;

	UFUNCTION()
	void OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, const bool bSuccess, FText Error,
	                                         ECommonUserPrivilege RequestedPrivilege,
	                                         ECommonUserOnlineContext OnlineContext);
};