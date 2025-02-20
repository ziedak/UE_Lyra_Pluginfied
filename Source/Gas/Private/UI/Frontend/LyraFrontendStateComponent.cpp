// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/LyraFrontendStateComponent.h"

#include "CommonGameInstance.h"
#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "ControlFlowManager.h"
#include "Experience/ExperienceManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NativeGameplayTags.h"
#include "PrimaryGameLayout.h"
#include "Tags/BaseGameplayTags.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraFrontendStateComponent)

namespace FrontendTags
{
	UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PLATFORM_TRAIT_SINGLEONLINEUSER, "Platform.Trait.SingleOnlineUser");
	//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Menu");
	//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MENU, "UI.Layer.Game");
}

ULyraFrontendStateComponent::ULyraFrontendStateComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

void ULyraFrontendStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for the experience load to complete
	const AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	const auto ExperienceComponent = GameState->FindComponentByClass<UExperienceManagerComponent>();
	check(ExperienceComponent);

	// This delegate is on a component with the same lifetime as this one, so no need to unhook it in
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(
		FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}


bool ULyraFrontendStateComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (!bShouldShowLoadingScreen) return false;
	OutReason = TEXT("Frontend Flow Pending...");

	if (FrontEndFlow.IsValid())
	{
		const TOptional<FString> StepDebugName = FrontEndFlow->GetCurrentStepDebugName();
		if (StepDebugName.IsSet()) OutReason = StepDebugName.GetValue();
	}

	return true;
}

void ULyraFrontendStateComponent::OnExperienceLoaded(const UExperienceDefinition_DA* Experience)
{
	FControlFlow& Flow = FControlFlowStatics::Create(this, TEXT("FrontendFlow"))
	                     .QueueStep(
		                     TEXT("Wait For User Initialization"), this, &ThisClass::FlowStep_WaitForUserInitialization)
	                     .QueueStep(
		                     TEXT("Try Show Press Start Screen"), this, &ThisClass::FlowStep_TryShowPressStartScreen)
	                     .QueueStep(
		                     TEXT("Try Join Requested Session"), this, &ThisClass::FlowStep_TryJoinRequestedSession)
	                     .QueueStep(TEXT("Try Show Main Screen"), this, &ThisClass::FlowStep_TryShowMainScreen);

	Flow.ExecuteFlow();
	FrontEndFlow = Flow.AsShared();
}

void ULyraFrontendStateComponent::FlowStep_WaitForUserInitialization(FControlFlowNodeRef SubFlow)
{
	// If this was a hard disconnect, explicitly destroy all user and session state
	// TODO: Refactor the engine disconnect flow so it is more explicit about why it happened
	bool bWasHardDisconnect = false;
	const AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);

	if (ensure(GameInstance) && ensure(GameMode) && UGameplayStatics::HasOption(GameMode->OptionsString, TEXT("closed"))) bWasHardDisconnect = true;

	// Only reset users on hard disconnect
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
	if (ensure(UserSubsystem) && bWasHardDisconnect) UserSubsystem->ResetUserState();

	// Always reset sessions
	UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
	if (ensure(SessionSubsystem)) SessionSubsystem->CleanUpSessions();

	SubFlow->ContinueFlow();
}

void ULyraFrontendStateComponent::FlowStep_TryShowPressStartScreen(FControlFlowNodeRef SubFlow)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	if (IsFirstUserLoggedIn(UserSubsystem))
	{
		SubFlow->ContinueFlow();
		return;
	}
	// Check to see if the platform actually requires a 'Press Start' screen.  This is only
	// required on platforms where there can be multiple online users where depending on what player's
	// controller presses 'Start' establishes the player to actually log in to the game with.
	if (UserSubsystem->ShouldWaitForStartInput())
	{
		ShowPressStartScreen(SubFlow);
		return;
	}
	StartAutoLoginProcess(SubFlow, UserSubsystem);
}

bool ULyraFrontendStateComponent::IsFirstUserLoggedIn(const UCommonUserSubsystem* UserSubsystem) const
{
	// Check to see if the first player is already logged in, if they are, we can skip the press start screen.
	const UCommonUserInfo* FirstUserInfo = UserSubsystem->GetUserInfoForLocalPlayerIndex(0);
	if (!FirstUserInfo) return false;

	return FirstUserInfo->InitializationState == ECommonUserInitializationState::LoggedInLocalOnly ||
		FirstUserInfo->InitializationState == ECommonUserInitializationState::LoggedInOnline;
}

void ULyraFrontendStateComponent::StartAutoLoginProcess(const FControlFlowNodeRef& SubFlow, UCommonUserSubsystem* UserSubsystem)
{
	// Start the auto login process, this should finish quickly and will use the default input device id
	InProgressPressStartScreen = SubFlow;
	UserSubsystem->OnUserInitializeComplete.AddDynamic(this, &ThisClass::OnUserInitialized);
	UserSubsystem->TryToInitializeForLocalPlay(0, FInputDeviceId(), false);
}

void ULyraFrontendStateComponent::ShowPressStartScreen(FControlFlowNodeRef SubFlow)
{
	const auto RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this);
	if (!RootLayout) return;

	constexpr bool bSuspendInputUntilComplete = true;
	RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(
		UITags::LAYER_GAME, bSuspendInputUntilComplete, PressStartScreenClass,
		[this, SubFlow](const EAsyncWidgetLayerState State, const UCommonActivatableWidget* Screen){
			if (State == EAsyncWidgetLayerState::AfterPush)
			{
				bShouldShowLoadingScreen = false;
				Screen->OnDeactivated().AddWeakLambda(this, [this, SubFlow](){ SubFlow->ContinueFlow(); });
				return;
			}
			if (State == EAsyncWidgetLayerState::Canceled)
			{
				bShouldShowLoadingScreen = false;
				SubFlow->ContinueFlow();
			}
		});
}

void ULyraFrontendStateComponent::OnUserInitialized(const UCommonUserInfo* UserInfo, const bool bSuccess, FText Error,
                                                    ECommonUserPrivilege RequestedPrivilege,
                                                    ECommonUserOnlineContext OnlineContext)
{
	const FControlFlowNodePtr FlowToContinue = InProgressPressStartScreen;
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UCommonUserSubsystem* UserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();

	if (!(ensure(FlowToContinue.IsValid() && UserSubsystem && GameInstance))) return;

	UserSubsystem->OnUserInitializeComplete.RemoveDynamic(this, &ULyraFrontendStateComponent::OnUserInitialized);
	InProgressPressStartScreen.Reset();

	// On success continue flow normally
	if (bSuccess)
	{
		FlowToContinue->ContinueFlow();
		return;
	}

	// TODO: Just continue for now, could go to some sort of error screen
	FlowToContinue->ContinueFlow();
}

void ULyraFrontendStateComponent::FlowStep_TryJoinRequestedSession(FControlFlowNodeRef SubFlow)
{
	UCommonGameInstance* GameInstance = Cast<UCommonGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GameInstance || !GameInstance->GetRequestedSession() || !GameInstance->CanJoinRequestedSession())
	{
		SubFlow->ContinueFlow();
		return;
	}

	// Skip this step if we didn't start requesting a session join
	UCommonSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UCommonSessionSubsystem>();
	if (!(ensure(SessionSubsystem)))
	{
		SubFlow->ContinueFlow();
		return;
	}

	// Bind to session join completion to continue or cancel the flow
	// TODO:  Need to ensure that after session join completes, the server travel completes.
	OnJoinSessionCompleteEventHandle = SessionSubsystem->OnJoinSessionCompleteEvent.AddWeakLambda(
		this, [this, SubFlow, SessionSubsystem](const FOnlineResultInformation& Result){
			// Unbind delegate. SessionSubsystem is the object triggering this event, so it must still be valid.
			SessionSubsystem->OnJoinSessionCompleteEvent.Remove(OnJoinSessionCompleteEventHandle);
			OnJoinSessionCompleteEventHandle.Reset();

			if (Result.bWasSuccessful)
			{
				// No longer transitioning to the main menu
				SubFlow->CancelFlow();
				return;
			}

			// Proceed to the main menu
			SubFlow->ContinueFlow();
		});
	GameInstance->JoinRequestedSession();
}

void ULyraFrontendStateComponent::FlowStep_TryShowMainScreen(FControlFlowNodeRef SubFlow)
{
	UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this);
	if (!RootLayout) return;

	constexpr bool bSuspendInputUntilComplete = true;
	RootLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(
		UITags::LAYER_MENU, bSuspendInputUntilComplete, MainScreenClass,
		[this, SubFlow](const EAsyncWidgetLayerState State, UCommonActivatableWidget* Screen){
			if (State == EAsyncWidgetLayerState::AfterPush || State == EAsyncWidgetLayerState::Canceled)
			{
				bShouldShowLoadingScreen = false;
				SubFlow->ContinueFlow();
			}
		});
}