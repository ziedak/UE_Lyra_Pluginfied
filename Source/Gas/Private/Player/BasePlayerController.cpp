// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BasePlayerController.h"

#include "AbilitySystemGlobals.h"
#include "CommonInputSubsystem.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Player/BaseLocalPlayer.h"
#include "Player/BasePlayerState.h"
#include "Settings/LyraSettingsShared.h"
#include "UI/Hud/BaseHUD.h"
#include "LyraPlayerCameraManager.h"
#include "Tags/BaseGameplayTags.h"

#if WITH_RPC_REGISTRY
// #include "Tests/LyraGameplayRpcRegistrationComponent.h"
//#include "HttpServerModule.h"
#endif


#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BasePlayerController)

namespace Lyra::Input
{
	static auto ShouldAlwaysPlayForceFeedback = 0;
	static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("LyraPC.ShouldAlwaysPlayForceFeedback"),
	                                                                 ShouldAlwaysPlayForceFeedback,
	                                                                 TEXT(
		                                                                 "Should force feedback effects be played, even if the last input device was not a gamepad?"));
}

ABasePlayerController::ABasePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ALyraPlayerCameraManager::StaticClass();
}

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	// #if WITH_RPC_REGISTRY
	// 	FHttpServerModule::Get().StartAllListeners();
	// 	int32 RpcPort = 0;
	// 	if (FParse::Value(FCommandLine::Get(), TEXT("rpcport="), RpcPort))
	// 	{
	// 		ULyraGameplayRpcRegistrationComponent* ObjectInstance = ULyraGameplayRpcRegistrationComponent::GetInstance();
	// 		if (ObjectInstance && ObjectInstance->IsValidLowLevel())
	// 		{
	// 			ObjectInstance->RegisterAlwaysOnHttpCallbacks();
	// 			ObjectInstance->RegisterInMatchHttpCallbacks();
	// 		}
	// 	}
	// #endif
	SetActorHiddenInGame(false);
}

void ABasePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Disable replicating the PC target view as it doesn't work well for replays or client-side spectating.
	// The engine TargetViewRotation is only set in APlayerController::TickActor if the server knows ahead of time that 
	// a specific pawn is being spectated and it only replicates down for COND_OwnerOnly.
	// In client-saved replays, COND_OwnerOnly is never true and the target pawn is not always known at the time of recording.
	// To support client-saved replays, the replication of this was moved to ReplicatedViewRotation and updated in PlayerTick.
	DISABLE_REPLICATED_PROPERTY(APlayerController, TargetViewRotation);
}

void ABasePlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);
	const UBaseLocalPlayer* BaseLocalPlayer = Cast<UBaseLocalPlayer>(InPlayer);
	if (!BaseLocalPlayer)
		return;

	// Register for settings changes
	ULyraSettingsShared* UserSettings = BaseLocalPlayer->GetSharedSettings();
	UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);

	OnSettingsChanged(UserSettings);
}

void ABasePlayerController::OnSettingsChanged(const ULyraSettingsShared* InSettings)
{
	bForceFeedbackEnabled = InSettings->GetForceFeedbackEnabled();
}

void ABasePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// If we are auto running then add some player input
	if (GetIsAutoRunning())
	{
		if (APawn* CurrentPawn = GetPawn())
		{
			const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			CurrentPawn->AddMovementInput(MovementDirection, 1.0f);
		}
	}


	if (!PlayerCameraManager)
		return;

	const APawn* TargetPawn = PlayerCameraManager->GetViewTargetPawn();
	if (!TargetPawn)
		return;

	auto Ps = GetBasePlayerState();
	if (!Ps)
		return;
	// Update view rotation on the server so it replicates
	if (HasAuthority() || TargetPawn->IsLocallyControlled())
		Ps->SetReplicatedViewRotation(TargetPawn->GetViewRotation());


	// Update the target view rotation if the pawn isn't locally controlled
	if (TargetPawn->IsLocallyControlled())
		return;

	Ps = TargetPawn->GetPlayerState<ABasePlayerState>();
	if (!Ps)
		return;
	// Get it from the spectated pawn's player state, which may not be the same as the PC's player state
	TargetViewRotation = Ps->GetReplicatedViewRotation();
}

void ABasePlayerController::SmoothTargetViewRotation(APawn* TargetPawn, float DeltaSeconds)
{
	// Default behavior is to interpolate to TargetViewRotation which is set from APlayerController::TickActor but it's not very smooth

	Super::SmoothTargetViewRotation(TargetPawn, DeltaSeconds);
}

void ABasePlayerController::UpdateHiddenComponents(const FVector& ViewLocation, TSet<FPrimitiveComponentId>& OutHiddenComponents)
{
	Super::UpdateHiddenComponents(ViewLocation, OutHiddenComponents);

	if (!bHideViewTargetPawnNextFrame)
		return;
	const AActor* ViewTargetPawn = PlayerCameraManager ? Cast<AActor>(PlayerCameraManager->GetViewTarget()) : nullptr;
	if (ViewTargetPawn)
	{
		// internal helper func to hide all the components
		auto AddToHiddenComponents = [&OutHiddenComponents](const TInlineComponentArray<UPrimitiveComponent*>& InComponents)
		{
			// add every component and all attached children
			for (const UPrimitiveComponent* Comp : InComponents)
			{
				if (!Comp->IsRegistered())
					continue;
				OutHiddenComponents.Add(Comp->GetPrimitiveSceneId());

				for (USceneComponent* AttachedChild : Comp->GetAttachChildren())
				{
					static FName NAME_NoParentAutoHide(TEXT("NoParentAutoHide"));
					const UPrimitiveComponent* AttachChildPC = Cast<UPrimitiveComponent>(AttachedChild);
					if (!AttachChildPC || !AttachChildPC->IsRegistered() || AttachChildPC->ComponentTags.Contains(NAME_NoParentAutoHide))
						continue;
					OutHiddenComponents.Add(AttachChildPC->GetPrimitiveSceneId());
				}
			}
		};

		//TODO Solve with an interface.  Gather hidden components or something.
		//TODO Hiding isn't awesome, sometimes you want the effect of a fade out over a proximity, needs to bubble up to designers.

		// hide pawn's components
		TInlineComponentArray<UPrimitiveComponent*> PawnComponents;
		ViewTargetPawn->GetComponents(PawnComponents);
		AddToHiddenComponents(PawnComponents);

		//// hide weapon too
		//if (ViewTargetPawn->CurrentWeapon)
		//{
		//	TInlineComponentArray<UPrimitiveComponent*> WeaponComponents;
		//	ViewTargetPawn->CurrentWeapon->GetComponents(WeaponComponents);
		//	AddToHiddenComponents(WeaponComponents);
		//}
	}

	// we consumed it, reset for next frame
	bHideViewTargetPawnNextFrame = false;
}

ABasePlayerState* ABasePlayerController::GetBasePlayerState() const { return CastChecked<ABasePlayerState>(PlayerState, ECastCheckedType::NullAllowed); }

UBaseAbilitySystemComponent* ABasePlayerController::GetBaseAbilitySystemComponent() const
{
	const ABasePlayerState* BasePs = GetBasePlayerState();
	return BasePs ? BasePs->GetBaseAbilitySystemComponent() : nullptr;
}

ABaseHud* ABasePlayerController::GetBaseHUD() const { return CastChecked<ABaseHud>(GetHUD(), ECastCheckedType::NullAllowed); }

void ABasePlayerController::SetIsAutoRunning(const bool bEnabled)
{
	if (bEnabled == GetIsAutoRunning())
		return;
	!bEnabled ? OnEndAutoRun() : OnStartAutoRun();
}


bool ABasePlayerController::GetIsAutoRunning() const
{
	auto bIsAutoRunning = false;
	if (const UBaseAbilitySystemComponent* LyraASC = GetBaseAbilitySystemComponent())
		bIsAutoRunning = LyraASC->GetTagCount(StatusTags::AUTORUNNING) > 0;
	return bIsAutoRunning;
}

void ABasePlayerController::OnStartAutoRun()
{
	if (UBaseAbilitySystemComponent* LyraASC = GetBaseAbilitySystemComponent())
	{
		LyraASC->SetLooseGameplayTagCount(StatusTags::AUTORUNNING, 1);
		K2_OnStartAutoRun();
	}
}

void ABasePlayerController::OnEndAutoRun()
{
	if (UBaseAbilitySystemComponent* LyraASC = GetBaseAbilitySystemComponent())
	{
		LyraASC->SetLooseGameplayTagCount(StatusTags::AUTORUNNING, 0);
		K2_OnEndAutoRun();
	}
}

void ABasePlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent())
		BaseAsc->ProcessAbilityInput(DeltaTime, bGamePaused);

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//TODO  implement this
	// #if WITH_SERVER_CODE && WITH_EDITOR
	// 	if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	// 	{
	// 		for (const FLyraCheatToRun& CheatRow : GetDefault<ULyraDeveloperSettings>()->CheatsToRun)
	// 		{
	// 			if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession)
	// 			{
	// 				ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
	// 			}
	// 		}
	// 	}
	// #endif

	SetIsAutoRunning(false);
}

void ABasePlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	// This can happen if the player controller is destroyed before the pawn
	const APawn* PawnBeingUnpossessed = GetPawn();
	if (!PawnBeingUnpossessed)
		return;
	UAbilitySystemComponent* Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState);
	if (!Asc)
		return;
	// If the ASC is still using the pawn as its avatar actor, clear it
	if (Asc->GetAvatarActor() == PawnBeingUnpossessed)
		Asc->SetAvatarActor(nullptr);

	Super::OnUnPossess();
}

void ABasePlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ABasePlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ABasePlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();
}

void ABasePlayerController::BroadcastOnPlayerStateChanged() { OnPlayerStateChanged(); }

void ABasePlayerController::OnPlayerStateChanged()
{
	// Empty, place for derived classes to implement without having to hook all the other events
}

void ABasePlayerController::UpdateForceFeedback(IInputInterface* InputInterface, const int32 ControllerId)
{
	if (bForceFeedbackEnabled)
	{
		if (const UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(GetLocalPlayer()))
		{
			const ECommonInputType CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
			if (Lyra::Input::ShouldAlwaysPlayForceFeedback ||
				CurrentInputType == ECommonInputType::Gamepad ||
				CurrentInputType == ECommonInputType::Touch)
			{
				InputInterface->SetForceFeedbackChannelValues(ControllerId, ForceFeedbackValues);
				return;
			}
		}
	}

	InputInterface->SetForceFeedbackChannelValues(ControllerId, FForceFeedbackValues());
}
