// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GGameMode.h"

#include "Character/BaseCharacter.h"
#include "Character/Components/PawnExtensionComponent.h"
#include "Core/GAssetManager.h"
#include "Core/GGameState.h"
#include "Core/GGameSession.h"
#include "Player/BasePlayerController.h"
#include "Player/BasePlayerState.h"
#include "Data/GasPawnData.h"
#include "Experience/ExperienceManagerComponent.h"
#include "Experience/DataAsset/ExperienceDefinition_DA.h"
#include "GameFramework/GameStateBase.h"


AGGameMode::AGGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = AGGameState::StaticClass();
	GameSessionClass = AGGameSession::StaticClass();
	PlayerControllerClass = ABasePlayerController::StaticClass();
	//ReplaySpectatorPlayerControllerClass = ABaseReplayPlayerController::StaticClass();
	PlayerStateClass = ABasePlayerState::StaticClass();
	DefaultPawnClass = ABaseCharacter::StaticClass();
	HUDClass = ABaseHud::StaticClass();
}

const UGasPawnData* AGGameMode::GetPawnDataForController(const AController* InController) const
{
	// See if pawn data is already set on the player state
	if (InController)
	{
		if (const ABasePlayerState* BasePS = InController->GetPlayerState<ABasePlayerState>())
		{
			if (const UGasPawnData* PawnData = BasePS->GetPawnData<UGasPawnData>()) { return PawnData; }
		}
	}

	// If not, fall back to the default for the current experience
	check(GameState);
	const UExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<
		UExperienceManagerComponent>();
	check(ExperienceComponent);

	// Experience not loaded yet, so there is no pawn data to be had
	if (!ExperienceComponent->IsExperienceLoaded()) { return nullptr; }

	const UExperienceDefinition_DA* Experience = ExperienceComponent->GetCurrentExperienceChecked();
	if (Experience->DefaultPawnData) { return static_cast<const UGasPawnData*>(Experience->DefaultPawnData); }

	//----- Experience is loaded and there's still no pawn data, fall back to the default for now
	return UGAssetManager::Get().GetDefaultPawnData();
}

APawn* AGGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
                                                              const FTransform& SpawnTransform)
{
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
		return nullptr;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient; // Never save the default player pawns into a map.
	SpawnInfo.bDeferConstruction = true;

	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
	if (!SpawnedPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."),
		       *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		return nullptr;
	}

	if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
	{
		if (const UGasPawnData* PawnData = GetPawnDataForController(NewPlayer)) { PawnExtComp->SetPawnData(PawnData); }
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."),
			       *GetNameSafe(SpawnedPawn));
		}
	}

	SpawnedPawn->FinishSpawning(SpawnTransform);

	return SpawnedPawn;
}


UClass* AGGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const UGasPawnData* PawnData = GetPawnDataForController(InController);
	if (PawnData && PawnData->PawnClass) { return PawnData->PawnClass; }

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}
