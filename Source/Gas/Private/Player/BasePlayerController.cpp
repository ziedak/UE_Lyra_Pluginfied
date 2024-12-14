// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BasePlayerController.h"

#include "AbilitySystemGlobals.h"
#include "CommonInputSubsystem.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Player/BaseLocalPlayer.h"
#include "Player/BasePlayerState.h"
#include "Settings/LyraSettingsShared.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BasePlayerController)

namespace Lyra::Input
{
	static int32 ShouldAlwaysPlayForceFeedback = 0;
	static FAutoConsoleVariableRef CVarShouldAlwaysPlayForceFeedback(TEXT("LyraPC.ShouldAlwaysPlayForceFeedback"),
	                                                                 ShouldAlwaysPlayForceFeedback,
	                                                                 TEXT(
		                                                                 "Should force feedback effects be played, even if the last input device was not a gamepad?"));
}

// Sets default values
ABasePlayerController::ABasePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(false);
}

void ABasePlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);
	if (const UBaseLocalPlayer* BaseLocalPlayer = Cast<UBaseLocalPlayer>(InPlayer))
	{
		ULyraSettingsShared* UserSettings = BaseLocalPlayer->GetSharedSettings();
		UserSettings->OnSettingChanged.AddUObject(this, &ThisClass::OnSettingsChanged);

		OnSettingsChanged(UserSettings);
	}
}

void ABasePlayerController::OnSettingsChanged(ULyraSettingsShared* InSettings)
{
	bForceFeedbackEnabled = InSettings->GetForceFeedbackEnabled();
}

ABasePlayerState* ABasePlayerController::GetBasePlayerState() const
{
	return CastChecked<ABasePlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UBaseAbilitySystemComponent* ABasePlayerController::GetBaseAbilitySystemComponent() const
{
	const ABasePlayerState* BasePS = GetBasePlayerState();
	return BasePS ? BasePS->GetBaseAbilitySystemComponent() : nullptr;
}


void ABasePlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (UBaseAbilitySystemComponent* BaseAsc = GetBaseAbilitySystemComponent())
	{
		BaseAsc->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ABasePlayerController::OnUnPossess()
{
	// Make sure the pawn that is being unpossessed doesn't remain our ASC's avatar actor
	if (const APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* Asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (Asc->GetAvatarActor() == PawnBeingUnpossessed)
			{
				Asc->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}

void ABasePlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void ABasePlayerController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();
}

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
