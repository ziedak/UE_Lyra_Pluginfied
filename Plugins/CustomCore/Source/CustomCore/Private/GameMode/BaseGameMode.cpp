// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BaseGameMode.h"

// #include "Character/BaseCharacter.h"
// #include "Character/Components/PawnExtensionComponent.h"
// #include "Player/BasePlayerController.h"
// #include "Player/BasePlayerState.h"
#include "AssetManager/BaseAssetManager.h"
// #include "Data/GasPawnData.h"

#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "GameMapsSettings.h"
#include "Development/CoreDeveloperSettings.h"
#include "Experience/ExperienceManagerComponent.h"
#include "Experience/DataAsset/ExperienceDefinition_DA.h"
#include "Experience/DataAsset/UserFacingExperienceDefinition_DA.h"
#include "GameMode/BaseWorldSettings.h"
#include "GameState/PlayerSpawningManagerComponent.h"
#include "Interface/IBotControllerInterface.h"

#include "Kismet/GameplayStatics.h"
#include "Log/Loggger.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGameMode)


// ABaseGameMode::ABaseGameMode(const FObjectInitializer& ObjectInitializer)
// 	: Super(ObjectInitializer)
// {
// 	//GameStateClass = ABaseGameState::StaticClass();
// 	//GameSessionClass = ABaseGameSession::StaticClass();
// 	PlayerControllerClass = ABasePlayerController::StaticClass();
// 	//ReplaySpectatorPlayerControllerClass = ABaseReplayPlayerController::StaticClass();
// 	PlayerStateClass = ABasePlayerState::StaticClass();
// 	DefaultPawnClass = ABaseCharacter::StaticClass();
// 	//HUDClass = ABaseHUD::StaticClass();
// }

// const UGasPawnData* ABaseGameMode::GetPawnDataForController(const AController* InController) const
// {
// 	// See if pawn data is already set on the player state
// 	if (InController)
// 	{
// 		if (const ABasePlayerState* BasePS = InController->GetPlayerState<ABasePlayerState>())
// 		{
// 			if (const UGasPawnData* PawnData = BasePS->GetPawnData<UGasPawnData>())
// 			{
// 				return PawnData;
// 			}
// 		}
// 	}
//
// 	// // If not, fall back to the the default for the current experience
// 	// check(GameState);
// 	// UExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
// 	// check(ExperienceComponent);
// 	//
// 	// if (ExperienceComponent->IsExperienceLoaded())
// 	// {
// 	// 	const UExperienceDefinition* Experience = ExperienceComponent->GetCurrentExperienceChecked();
// 	// 	if (Experience->DefaultPawnData != nullptr)
// 	// 	{
// 	// 		return Experience->DefaultPawnData;
// 	// 	}
// 	//
// 	// 	//----- Experience is loaded and there's still no pawn data, fall back to the default for now
// 	return UBaseAssetManager::Get().GetDefaultPawnData();
// 	// }
//
// 	// Experience not loaded yet, so there is no pawn data to be had
// 	return nullptr;
// }

ABaseGameMode::ABaseGameMode(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
}

void ABaseGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	// Wait for the next frame to give time to initialize startup settings
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}

// void ABaseGameMode::HandleMatchAssignmentIfNotExpectingOne()
// {
// 	FPrimaryAssetId ExperienceId;
// 	FString ExperienceIdSource;
//
// 	// Precedence order (highest wins)
// 	//  - Matchmaking assignment (if present)
// 	//  - URL Options override
// 	//  - Developer Settings (PIE only)
// 	//  - Command Line override
// 	//  - World Settings
// 	//  - Dedicated server
// 	//  - Default experience
//
// 	UWorld* World = GetWorld();
//
// 	if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
// 	{
// 		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
// 		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExperienceDefinition_DA::StaticClass()->GetFName()), FName(*ExperienceFromOptions));
// 		ExperienceIdSource = TEXT("OptionsString");
// 	}
//
// 	if (!ExperienceId.IsValid() && World->IsPlayInEditor())
// 	{
// 		ExperienceId = GetDefault<UDeveloperSettings>()->ExperienceOverride;
// 		ExperienceIdSource = TEXT("DeveloperSettings");
// 	}
//
// 	// see if the command line wants to set the experience
// 	if (!ExperienceId.IsValid())
// 	{
// 		FString ExperienceFromCommandLine;
// 		if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
// 		{
// 			ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
// 			if (!ExperienceId.PrimaryAssetType.IsValid())
// 			{
// 				ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExperienceDefinition_DA::StaticClass()->GetFName()), FName(*ExperienceFromCommandLine));
// 			}
// 			ExperienceIdSource = TEXT("CommandLine");
// 		}
// 	}
//
// 	// see if the world settings has a default experience
// 	if (!ExperienceId.IsValid())
// 	{
// 		if (AWorldSettings* TypedWorldSettings = Cast<AWorldSettings>(GetWorldSettings()))
// 		{
// 			ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
// 			ExperienceIdSource = TEXT("WorldSettings");
// 		}
// 	}
//
// 	UAssetManager& AssetManager = UAssetManager::Get();
// 	FAssetData Dummy;
// 	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
// 	{
// 		UE_LOG(LogExperience, Error, TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"), *ExperienceId.ToString());
// 		ExperienceId = FPrimaryAssetId();
// 	}
//
// 	// Final fallback to the default experience
// 	if (!ExperienceId.IsValid())
// 	{
// 		if (TryDedicatedServerLogin())
// 		{
// 			// This will start to host as a dedicated server
// 			return;
// 		}
//
// 		//@TODO: Pull this from a config setting or something
// 		ExperienceId = FPrimaryAssetId(FPrimaryAssetType("ExperienceDefinition"), FName("B_DefaultExperience"));
// 		ExperienceIdSource = TEXT("Default");
// 	}
//
// 	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
// }

void ABaseGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
	// Precedence order (highest wins)
	//  - Matchmaking assignment (if present)
	//  - URL Options override
	//  - Developer Settings (PIE only)
	//  - Command Line override
	//  - World Settings
	//  - Dedicated server
	//  - Default experience

	FString ExperienceIdSource = TEXT("Not_Found");
	UWorld* World = GetWorld();
	check(World);

	FPrimaryAssetId ExperienceId = GetExperienceFromOptions(ExperienceIdSource);
	if (!ExperienceId.IsValid())
	{
		ExperienceId = GetExperienceFromEditor(World, ExperienceIdSource);
	}
	if (!ExperienceId.IsValid())
	{
		ExperienceId = GetExperienceFromCommandLine(ExperienceIdSource);
	}
	if (!ExperienceId.IsValid())
	{
		ExperienceId = GetExperienceFromWorldSettings(World, ExperienceIdSource);
	}
	if (!ExperienceId.IsValid())
	{
		ExperienceId = GetDefaultExperience(ExperienceIdSource);
	}

	check(ExperienceId.IsValid());
	if (!ValidateExperienceAssetData(ExperienceId))
	{
		ExperienceId = FPrimaryAssetId();
	}

	OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}

FPrimaryAssetId ABaseGameMode::GetExperienceFromOptions(FString& ExperienceIdSource) const
{
	FPrimaryAssetId ExperienceId;
	if (UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
	{
		const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExperienceDefinition_DA::StaticClass()->GetFName()),
		                               FName(*ExperienceFromOptions));
		ExperienceIdSource = TEXT("OptionsString");
	}
	return ExperienceId;
}

FPrimaryAssetId ABaseGameMode::GetExperienceFromEditor(const UWorld* World, FString& ExperienceIdSource) const
{
	FPrimaryAssetId ExperienceId;
	if (World->IsPlayInEditor())
	{
		ExperienceId = GetDefault<UCoreDeveloperSettings>()->ExperienceOverride;
		ExperienceIdSource = TEXT("DeveloperSettings");
	}
	return ExperienceId;
}

FPrimaryAssetId ABaseGameMode::GetExperienceFromCommandLine(FString& ExperienceIdSource) const
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceFromCommandLine;
	if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
	{
		ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
		if (!ExperienceId.PrimaryAssetType.IsValid())
		{
			ExperienceId = FPrimaryAssetId(FPrimaryAssetType(UExperienceDefinition_DA::StaticClass()->GetFName()),
			                               FName(*ExperienceFromCommandLine));
		}
		ExperienceIdSource = TEXT("CommandLine");
	}
	return ExperienceId;
}

FPrimaryAssetId ABaseGameMode::GetExperienceFromWorldSettings(UWorld* World, FString& ExperienceIdSource) const
{
	FPrimaryAssetId ExperienceId;
	if (const ABaseWorldSettings* TypedWorldSettings = Cast<ABaseWorldSettings>(GetWorldSettings()))
	{
		ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
		ExperienceIdSource = TEXT("WorldSettings");
	}
	return ExperienceId;
}

FPrimaryAssetId ABaseGameMode::GetDefaultExperience(FString& ExperienceIdSource)
{
	FPrimaryAssetId ExperienceId;
	if (TryDedicatedServerLogin())
	{
		// This will start to host as a dedicated server
		return ExperienceId;
	}

	ExperienceId = FPrimaryAssetId(FPrimaryAssetType("ExperienceDefinition_DA"), FName("B_DefaultExperience"));
	ExperienceIdSource = TEXT("Default");
	return ExperienceId;
}

bool ABaseGameMode::ValidateExperienceAssetData(const FPrimaryAssetId& ExperienceId) const
{
	const UAssetManager& AssetManager = UAssetManager::Get();
	FAssetData Dummy;
	if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, /*out*/ Dummy))
	{
		UE_LOG(LogExperience, Error,
		       TEXT("EXPERIENCE: Wanted to use %s but couldn't find it, falling back to the default)"),
		       *ExperienceId.ToString());
		return false;
	}
	return true;
}

// UClass* ABaseGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
// {
// 	if (const UGasPawnData* PawnData = GetPawnDataForController(InController))
// 	{
// 		if (PawnData->PawnClass)
// 		{
// 			return PawnData->PawnClass;
// 		}
// 	}
// 	return Super::GetDefaultPawnClassForController_Implementation(InController);
// }

// APawn* ABaseGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
//                                                                  const FTransform& SpawnTransform)
// {
// 	
// 	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
// 	if (!PawnClass)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
// 		return nullptr;
// 	}
//
// 	FActorSpawnParameters SpawnInfo;
// 	SpawnInfo.Instigator = GetInstigator();
// 	SpawnInfo.ObjectFlags |= RF_Transient; // Never save the default player pawns into a map.
// 	SpawnInfo.bDeferConstruction = true;
// 	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
// 	if (!SpawnedPawn)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."),
// 		       *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
// 		return nullptr;
// 	}
//
// 	if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
// 	{
// 		const UGasPawnData* PawnData = GetPawnDataForController(NewPlayer);
// 		if (!PawnData)
// 		{
// 			UE_LOG(LogTemp, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."),
// 			       *GetNameSafe(SpawnedPawn));
// 		}
// 		else
// 		{
// 			PawnExtComp->SetPawnData(PawnData);
// 		}
// 	}
//
// 	SpawnedPawn->FinishSpawning(SpawnTransform);
//
// 	return SpawnedPawn;
// }


void ABaseGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Delay starting new players until the experience has been loaded
	// (players who log in prior to that will be started by OnExperienceLoaded)
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

AActor* ABaseGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (const auto PlayerSpawningComponent = GameState->FindComponentByClass<
		UPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ChoosePlayerStart(Player);
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void ABaseGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	if (const auto PlayerSpawningComponent = GameState->FindComponentByClass<
		UPlayerSpawningManagerComponent>())
	{
		PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}

	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool ABaseGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return ControllerCanRestart(Player);
}

bool ABaseGameMode::ControllerCanRestart(AController* Controller)
{
	// Bot version of Super::PlayerCanRestart_Implementation
	if (!Controller || Controller->IsPendingKillPending())
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC && !Super::PlayerCanRestart_Implementation(PC))
	{
		return false;
	}
	if (const auto PlayerSpawningComponent = GameState->FindComponentByClass<
		UPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningComponent->ControllerCanRestart(Controller);
	}


	return true;
}

void ABaseGameMode::InitGameState()
{
	Super::InitGameState();
	// Listen for the experience load to complete	
	UExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded(
		FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void ABaseGameMode::OnExperienceLoaded(const UExperienceDefinition_DA* CurrentExperience)
{
	// Spawn any players that are already attached
	//@TODO: Here we're handling only *player* controllers, but in GetDefaultPawnClassForController_Implementation we skipped all controllers
	// GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if (PC && !PC->GetPawn() && PlayerCanRestart(PC))
		{
			RestartPlayer(PC);
		}
	}
}

bool ABaseGameMode::IsExperienceLoaded() const
{
	check(GameState);
	const auto ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceComponent);

	return ExperienceComponent->IsExperienceLoaded();
}


void ABaseGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void ABaseGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	// If we tried to spawn a pawn and it failed, lets try again *note* check if there's actually a pawn class
	// before we try this forever.
	if (const auto PawnClass = GetDefaultPawnClassForController(NewPlayer); !PawnClass)
	{
		UE_LOG(LogExperience, Verbose, TEXT("FailedToRestartPlayer(%s) but there's no pawn class so giving up."),
		       *GetPathNameSafe(NewPlayer));
		return;
	}
	const auto NewPC = Cast<APlayerController>(NewPlayer);
	if (!NewPC)
	{
		RequestPlayerRestartNextFrame(NewPlayer, false);
		return;
	}

	// If it's a player don't loop forever, maybe something changed and they can no longer restart if so stop trying.
	if (!PlayerCanRestart(NewPC))
	{
		UE_LOG(LogExperience, Verbose,
		       TEXT("FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again."),
		       *GetPathNameSafe(NewPlayer));
		return;
	}

	RequestPlayerRestartNextFrame(NewPlayer, false);
}


void ABaseGameMode::RequestPlayerRestartNextFrame(AController* Controller, const bool bForceReset)
{
	if (bForceReset && Controller)
	{
		Controller->Reset();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
		return;
	}

	// if (auto BotController = Cast<APlayerBotController>(Controller))
	// 	GetWorldTimerManager().SetTimerForNextTick(BotController, &APlayerBotController::ServerRestartController);


	//TODO:Check this
	GetWorldTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &ThisClass::ServerRestartBot, Controller));
}

void ABaseGameMode::ServerRestartBot(AController* Controller) const
{
	if (Controller && Controller->Implements<UBotControllerInterface>())
	{
		if (const auto IBotController = Cast<IBotControllerInterface>(Controller))
		{
			IBotController->ServerRestartController();
		}
	}
}

void ABaseGameMode::OnMatchAssignmentGiven(const FPrimaryAssetId& ExperienceId, const FString& ExperienceIdSource) const
{
	if (!ExperienceId.IsValid())
	{
		UE_LOG(LogExperience, Error, TEXT("Failed to identify experience, loading screen will stay up forever"));
		return;
	}

	UE_LOG(LogExperience, Log, TEXT("Identified experience %s (Source: %s)"), *ExperienceId.ToString(),
	       *ExperienceIdSource);

	UExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->SetCurrentExperience(ExperienceId);
}


bool ABaseGameMode::TryDedicatedServerLogin()
{
	// Some basic code to register as an active dedicated server, this would be heavily modified by the game
	FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || !World || World->GetNetMode() != NM_DedicatedServer || World->URL.Map != DefaultMap)
	{
		return false;
	}
	// Only register if this is the default map on a dedicated server
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	// Dedicated servers may need to do an online login
	UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ThisClass::OnUserInitializedForDedicatedServer);

	// There are no local users on dedicated server, but index 0 means the default platform user which is handled by the online login code
	if (!UserSubsystem->TryToLoginForOnlinePlay(0))
	{
		OnUserInitializedForDedicatedServer(nullptr,
		                                    false,
		                                    FText(),
		                                    ECommonUserPrivilege::CanPlayOnline,
		                                    ECommonUserOnlineContext::Default);
	}

	return true;
}

void ABaseGameMode::OnUserInitializedForDedicatedServer(const UCommonUserInfo* UserInfo, const bool bSuccess,
                                                        FText Error,
                                                        ECommonUserPrivilege RequestedPrivilege,
                                                        ECommonUserOnlineContext OnlineContext)
{
	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// Unbind
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
	UserSubsystem->OnUserInitializeComplete.RemoveDynamic(this, &ThisClass::OnUserInitializedForDedicatedServer);

	// Dedicated servers do not require user login, but some online subsystems may expect it
	if (bSuccess && ensure(UserInfo))
	{
		UE_LOG(LogExperience, Log, TEXT("Dedicated server user login succeeded for id %s, starting online server"),
		       *UserInfo->GetNetId().ToString());
	}
	else
	{
		UE_LOG(LogExperience, Log,
		       TEXT("Dedicated server user login unsuccessful, starting online server as login is not required"));
	}

	HostDedicatedServerMatch(ECommonSessionOnlineMode::Online);
}

// void ABaseGameMode::HostDedicatedServerMatch(const ECommonSessionOnlineMode OnlineMode) const
// {
// 	FPrimaryAssetType UserExperienceType = UUserFacingExperienceDefinition_DA::StaticClass()->GetFName();
//
// 	// Figure out what UserFacingExperience to load
// 	FPrimaryAssetId UserExperienceId;
// 	FString UserExperienceFromCommandLine;
// 	if (FParse::Value(FCommandLine::Get(), TEXT("UserExperience="), UserExperienceFromCommandLine) ||
// 		FParse::Value(FCommandLine::Get(), TEXT("Playlist="), UserExperienceFromCommandLine))
// 	{
// 		UserExperienceId = FPrimaryAssetId::ParseTypeAndName(UserExperienceFromCommandLine);
// 		if (!UserExperienceId.PrimaryAssetType.IsValid())
// 			UserExperienceId = FPrimaryAssetId(FPrimaryAssetType(UserExperienceType),
// 			                                   FName(*UserExperienceFromCommandLine));
// 	}
//
// 	// Search for the matching experience, it's fine to force load them because we're in dedicated server startup
// 	UAssetManager& AssetManager = UAssetManager::Get();
// 	TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAssetsWithType(UserExperienceType);
// 	if (ensure(Handle.IsValid()))
// 		Handle->WaitUntilComplete();
//
// 	TArray<UObject*> UserExperiences;
// 	AssetManager.GetPrimaryAssetObjectList(UserExperienceType, UserExperiences);
// 	UUserFacingExperienceDefinition_DA* FoundExperience = nullptr;
// 	UUserFacingExperienceDefinition_DA* DefaultExperience = nullptr;
//
// 	for (UObject* Object : UserExperiences)
// 	{
// 		UUserFacingExperienceDefinition_DA* UserExperience = Cast<UUserFacingExperienceDefinition_DA>(Object);
// 		if (!(ensure(UserExperience)))
// 			continue;
//
// 		if (UserExperience->GetPrimaryAssetId() == UserExperienceId)
// 		{
// 			FoundExperience = UserExperience;
// 			break;
// 		}
//
// 		if (UserExperience->bIsDefaultExperience && DefaultExperience == nullptr)
// 			DefaultExperience = UserExperience;
// 	}
//
// 	if (!FoundExperience)
// 		FoundExperience = DefaultExperience;
//
// 	UGameInstance* GameInstance = GetGameInstance();
// 	if (!ensure(FoundExperience && GameInstance))
// 		return;
//
// 	// Actually host the game
// 	UCommonSession_HostSessionRequest* HostRequest = FoundExperience->CreateHostingRequest(this);
// 	if (!(ensure(HostRequest)))
// 		return;
//
// 	HostRequest->OnlineMode = OnlineMode;
//
// 	// TODO override other parameters?
//
// 	UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
// 	SessionSubsystem->HostSession(nullptr, HostRequest);
//
// 	// This will handle the map travel
// }

void ABaseGameMode::HostDedicatedServerMatch(const ECommonSessionOnlineMode OnlineMode) const
{
	// Load user experience and host the game
	UUserFacingExperienceDefinition_DA* UserExperience = LoadUserExperience();
	if (!ensure(UserExperience))
	{
		return;
	}

	UCommonSession_HostSessionRequest* HostRequest = CreateHostSessionRequest(UserExperience, OnlineMode);
	if (!ensure(HostRequest))
	{
		return;
	}

	HostGameSession(HostRequest);
}

UUserFacingExperienceDefinition_DA* ABaseGameMode::LoadUserExperience() const
{
	const FPrimaryAssetType UserExperienceType = UUserFacingExperienceDefinition_DA::StaticClass()->GetFName();
	const FPrimaryAssetId UserExperienceId = GetUserExperienceIdFromCommandLine(UserExperienceType);

	UAssetManager& AssetManager = UAssetManager::Get();
	const TSharedPtr<FStreamableHandle> Handle = AssetManager.LoadPrimaryAssetsWithType(UserExperienceType);
	if (ensure(Handle.IsValid()))
	{
		Handle->WaitUntilComplete();
	}

	return FindUserExperience(UserExperienceType, UserExperienceId);
}

FPrimaryAssetId ABaseGameMode::GetUserExperienceIdFromCommandLine(const FPrimaryAssetType& UserExperienceType) const
{
	FPrimaryAssetId UserExperienceId;
	FString UserExperienceFromCommandLine;
	if (FParse::Value(FCommandLine::Get(), TEXT("UserExperience="), UserExperienceFromCommandLine) ||
	    FParse::Value(FCommandLine::Get(), TEXT("Playlist="), UserExperienceFromCommandLine))
	{
		UserExperienceId = FPrimaryAssetId::ParseTypeAndName(UserExperienceFromCommandLine);
		if (!UserExperienceId.PrimaryAssetType.IsValid())
		{
			UserExperienceId = FPrimaryAssetId(UserExperienceType, FName(*UserExperienceFromCommandLine));
		}
	}
	return UserExperienceId;
}

UUserFacingExperienceDefinition_DA* ABaseGameMode::FindUserExperience(const FPrimaryAssetType& UserExperienceType,
                                                                      const FPrimaryAssetId& UserExperienceId) const
{
	TArray<UObject*> UserExperiences;
	const UAssetManager& AssetManager = UAssetManager::Get();
	AssetManager.GetPrimaryAssetObjectList(UserExperienceType, UserExperiences);
	UUserFacingExperienceDefinition_DA* FoundExperience = nullptr;
	UUserFacingExperienceDefinition_DA* DefaultExperience = nullptr;

	for (UObject* Object : UserExperiences)
	{
		UUserFacingExperienceDefinition_DA* UserExperience = Cast<UUserFacingExperienceDefinition_DA>(Object);
		if (!ensure(UserExperience))
		{
			continue;
		}

		if (UserExperience->GetPrimaryAssetId() == UserExperienceId)
		{
			FoundExperience = UserExperience;
			break;
		}

		if (UserExperience->bIsDefaultExperience && DefaultExperience == nullptr)
		{
			DefaultExperience = UserExperience;
		}
	}

	return FoundExperience ? FoundExperience : DefaultExperience;
}

UCommonSession_HostSessionRequest* ABaseGameMode::CreateHostSessionRequest(
	const UUserFacingExperienceDefinition_DA* UserExperience, const ECommonSessionOnlineMode OnlineMode) const
{
	UCommonSession_HostSessionRequest* HostRequest = UserExperience->CreateHostingRequest(this);
	if (ensure(HostRequest))
	{
		HostRequest->OnlineMode = OnlineMode;
		// TODO: override other parameters if needed
	}
	return HostRequest;
}

void ABaseGameMode::HostGameSession(UCommonSession_HostSessionRequest* HostRequest) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	if (!ensure(GameInstance))
	{
		return;
	}

	UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
	SessionSubsystem->HostSession(nullptr, HostRequest);
	// This will handle the map travel
}
