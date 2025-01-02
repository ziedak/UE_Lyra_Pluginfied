// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/PlayerSpawningManagerComponent.h"

#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
#include "GameFramework/PlayerState.h"
#include "Interface/IPlayerSpawnInterface.h"
#include "GameFramework/Controller.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PlayerSpawningManagerComponent)

DEFINE_LOG_CATEGORY_STATIC(LogPlayerSpawning, Log, All);

UPlayerSpawningManagerComponent::UPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnLevelAdded);

	const UWorld* World = GetWorld();
	check(World);
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::HandleOnActorSpawned));

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (APlayerStart* PlayerStart = *It) { CachedPlayerStarts.Add(PlayerStart); }
	}
}

void UPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		for (AActor* Actor : InLevel->Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	if (APlayerStart* PlayerStart = Cast<APlayerStart>(SpawnedActor)) { CachedPlayerStarts.Add(PlayerStart); }
}

// ABaseGameMode Proxied Calls - Need to handle when someone chooses
// to restart a player the normal way in the engine.
//======================================================================

AActor* UPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (!Player) { return nullptr; }

#if WITH_EDITOR
	if (APlayerStart* PlayerStart = FindPlayFromHereStart(Player)) { return PlayerStart; }
#endif

	TArray<APlayerStart*> StarterPoints;
	for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
	{
		if (APlayerStart* Start = StartIt->Get())
		{
			StarterPoints.Add(Start);
			continue;
		}

		StartIt.RemoveCurrent();
	}

	if (const APlayerState* PlayerState = Player->GetPlayerState<APlayerState>())
	{
		// start dedicated spectators at any random starting location, but they do not claim it
		if (PlayerState->IsOnlyASpectator())
		{
			if (!StarterPoints.IsEmpty()) { return StarterPoints[FMath::RandRange(0, StarterPoints.Num() - 1)]; }

			return nullptr;
		}
	}

	AActor* PlayerStart = OnChoosePlayerStart(Player, StarterPoints);

	if (!PlayerStart) { PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player, StarterPoints); }


	if (const auto BaseStart = Cast<IPlayerSpawnInterface>(PlayerStart)) { BaseStart->TryClaim(Player); }


	// if (APlayerStart* BaseStart = Cast<APlayerStart>(PlayerStart))
	// 	BaseStart->TryClaim(Player);

	return PlayerStart;
}

#if WITH_EDITOR
APlayerStart* UPlayerSpawningManagerComponent::FindPlayFromHereStart(const AController* Player) const
{
	const UWorld* World = GetWorld();

	// Only 'Play From Here' for a player controller, bots etc. should all spawn from normal spawn points.
	if (!Player->IsA<APlayerController>() || !World) { return nullptr; }

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart && PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			return PlayerStart;
		}
	}
	return nullptr;
}
#endif

bool UPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player) const
{
	constexpr bool bCanRestart = true;

	// TODO Can they restart?

	return bCanRestart;
}

void UPlayerSpawningManagerComponent::OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation)
{
	//  Finish Restart Player
}

void UPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	OnFinishRestartPlayer(NewPlayer, StartRotation);
	K2_OnFinishRestartPlayer(NewPlayer, StartRotation);
}

APlayerStart* UPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(
	AController* Controller, const TArray<APlayerStart*>& FoundStartPoints) const
{
	if (!Controller) { return nullptr; }

	TArray<APlayerStart*> UnOccupiedStartPoints;
	TArray<APlayerStart*> OccupiedStartPoints;

	for (APlayerStart* StartPoint : FoundStartPoints)
	{
		//const EPlayerStartLocationOccupancy State = StartPoint->GetLocationOccupancy(Controller);

		EPlayerStartLocationOccupancy State = EPlayerStartLocationOccupancy::Full;
		if (const auto BaseStart = Cast<IPlayerSpawnInterface>(StartPoint))
		{
			State = BaseStart->GetLocationOccupancy(Controller);
		}

		if (State == EPlayerStartLocationOccupancy::Empty) { UnOccupiedStartPoints.Add(StartPoint); }

		if (State == EPlayerStartLocationOccupancy::Partial) { OccupiedStartPoints.Add(StartPoint); }
	}

	if (UnOccupiedStartPoints.Num() > 0)
	{
		return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
	}

	if (OccupiedStartPoints.Num() > 0)
	{
		return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
	}

	return nullptr;
}
