// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/HeroComponent.h"

#include "InputConfig/LyraInputComponent.h"

#include "Component/BaseAbilitySystemComponent.h"
#include "Character/Components/PawnExtensionComponent.h"
#include "Player/BasePlayerState.h"
#include "Tags/BaseGameplayTags.h"
#include "Data/GasPawnData.h"
#include "Log/Loggger.h"

#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFeatures/GameFeatureAction_AddInputContextMapping.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "GameFramework/PlayerState.h"

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR


#include "Player/BasePlayerController.h"
#include "Tags/CoreTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HeroComponent)

const FName UHeroComponent::NAME_BIND_INPUTS_NOW("BindInputsNow");
const FName UHeroComponent::NAME_ACTOR_FEATURE_NAME("Hero");

UHeroComponent::UHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  //  AbilityCameraMode(nullptr),
	  bReadyToBindInputs(false) {}

// void UHeroComponent::SetAbilityCameraMode(TSubclassOf<UBaseCameraMode> CameraMode,
// 	const FGameplayAbilitySpecHandle& OwningSpecHandle)
// {
//
// 	if (!CameraMode)
// 		return;
//
// 	AbilityCameraMode = CameraMode;
// 	AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
// }

// void UHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
// {
// }

void UHeroComponent::AddAdditionalInputConfig(const ULyraInputConfig_DA* InputConfig)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) { return; }

	// do we need those checks?
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	const auto Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	const auto PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	if (!PawnExtComp) { return; }

	const auto InputComponent = Pawn->FindComponentByClass<ULyraInputComponent>();
	if (!(ensureMsgf(InputComponent,
	                 TEXT(
		                 "Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs."
		                 " Change the input component to ULyraInputComponent or a subclass of it."
	                 )))) { return; }

	TArray<uint32> BindHandles;
	InputComponent->BindAbilityActionList(InputConfig,
	                                      this,
	                                      &ThisClass::Input_AbilityInputTagPressed,
	                                      &ThisClass::Input_AbilityInputTagReleased,
	                                      /*out*/ BindHandles);
}

void UHeroComponent::RemoveAdditionalInputConfig(const ULyraInputConfig_DA* InputConfig) const
{
	//@TODO: Implement me! what the heck is this function supposed to do?
}

#pragma region IGameFrameworkInitStateInterface

// bool UHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, const FGameplayTag CurrentState,
//                                         const FGameplayTag DesiredState) const
// {
// 	check(Manager);
// 	APawn* Pawn = GetPawn<APawn>();
//
// 	// As long as we have a real pawn, let us transition
// 	if (!CurrentState.IsValid() && DesiredState == InitStateTags::SPAWNED)
// 		return Pawn != nullptr;
//
// 	if (CurrentState == InitStateTags::SPAWNED && DesiredState == InitStateTags::DATA_AVAILABLE)
// 	{
// 		//The player state is required.
// 		if (!GetPlayerState<ABasePlayerState>())
// 			return false;
//
// 		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
// 		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
// 		{
// 			const AController* Controller = GetController<AController>();
// 			const bool bHasControllerPairedWithPlayerState = Controller && Controller->PlayerState && Controller->
// 				PlayerState->GetOwner() == Controller;
//
// 			if (!bHasControllerPairedWithPlayerState)
// 				return false;
// 		}
//
// 		// If we're not a bot, we need a player controller and a local player
// 		if (!Pawn->IsLocallyControlled() || Pawn->IsBotControlled())
// 			return true;
//
// 		const ABasePlayerController* PC = GetController<ABasePlayerController>();
// 		return Pawn->InputComponent && PC && PC->GetLocalPlayer();
// 	}
// 	 if (CurrentState == InitStateTags::DATA_AVAILABLE && DesiredState == InitStateTags::DATA_INITIALIZED)
// 	{
// 		// Wait for player state and extension component
// 		const ABasePlayerState* BasePS = GetPlayerState<ABasePlayerState>();
// 		const auto Current = Manager->GetInitStateForFeature(Pawn,
// 		                                                     UPawnExtensionComponent::Name_ActorFeatureName);
// 		ULOG_INFO(LogTemp, "======== Current State: %s", *Current.ToString());
// 		const bool PawnExtensionInitialized = Manager->HasFeatureReachedInitState(Pawn,
// 			UPawnExtensionComponent::Name_ActorFeatureName,
// 			InitStateTags::DATA_INITIALIZED);
// 		// const bool PawnExtensionReady = Manager->HasFeatureReachedInitState(Pawn,
// 		//                                                                     UPawnExtensionComponent::Name_ActorFeatureName,
// 		//                                                                     InitStateTags::GAMEPLAY_READY);
// 		// return BasePS && (PawnExtensionInitialized || PawnExtensionReady);
//
// 		return BasePS && PawnExtensionInitialized;
// 	}
// 	else if (CurrentState == InitStateTags::DATA_INITIALIZED && DesiredState == InitStateTags::GAMEPLAY_READY)
// 	{
// 		// TODO add ability initialization checks?
// 		return true;
//
// 		//GPT: Wait for the ability system component to be initialized
// 		// if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
// 		// {
// 		// 	if (UBaseAbilitySystemComponent* Asc = PawnExtComp->GetBaseAbilitySystemComponent())
// 		// 	{
// 		// 		return Manager->HasFeatureReachedInitState(Asc, UBaseAbilitySystemComponent::Name_ActorFeatureName, InitStateTags::GAMEPLAY_READY);
// 		// 	}
// 		// }
// 	}
// 	return false;
// 	//GPT:
// 	// else if (CurrentState == InitStateTags::GAMEPLAY_READY && DesiredState == InitStateTags::READY_TO_BIND_INPUTS)
// 	// {
// 	// 	// Wait for the ability system component to be initialized
// 	// 	if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
// 	// 	{
// 	// 		if (UBaseAbilitySystemComponent* Asc = PawnExtComp->GetBaseAbilitySystemComponent())
// 	// 		{
// 	// 			return Manager->HasFeatureReachedInitState(Asc, UBaseAbilitySystemComponent::Name_ActorFeatureName, InitStateTags::READY_TO_BIND_INPUTS);
// 	// 		}
// 	// 	}
// 	// }
// 	// else if (CurrentState == InitStateTags::READY_TO_BIND_INPUTS && DesiredState == InitStateTags::READY_TO_BIND_INPUTS)
// 	// {
// 	// 	// TODO add ability initialization checks?
// 	// 	return true;
// 	// }
// }


bool UHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager,
                                        const FGameplayTag CurrentState,
                                        const FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == InitStateTags::SPAWNED) { return CanTransitionToSpawned(Pawn); }

	if (CurrentState == InitStateTags::SPAWNED && DesiredState == InitStateTags::DATA_AVAILABLE)
	{
		return CanTransitionToDataAvailable(Pawn);
	}

	if (CurrentState == InitStateTags::DATA_AVAILABLE && DesiredState == InitStateTags::DATA_INITIALIZED)
	{
		return CanTransitionToDataInitialized(Manager, Pawn);
	}

	if (CurrentState == InitStateTags::DATA_INITIALIZED && DesiredState == InitStateTags::GAMEPLAY_READY)
	{
		return CanTransitionToGameplayReady();
	}

	return false;
}


bool UHeroComponent::CanTransitionToDataAvailable(const APawn* Pawn) const
{
	if (!GetPlayerState<ABasePlayerState>()) { return false; }

	if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
	{
		const AController* Controller = GetController<AController>();
		const bool bHasControllerPairedWithPlayerState = Controller && Controller->PlayerState && Controller->
			PlayerState->GetOwner() == Controller;

		if (!bHasControllerPairedWithPlayerState) { return false; }
	}

	if (!Pawn->IsLocallyControlled() || Pawn->IsBotControlled()) { return true; }

	const ABasePlayerController* PC = GetController<ABasePlayerController>();
	return Pawn->InputComponent && PC && PC->GetLocalPlayer();
}

bool UHeroComponent::CanTransitionToDataInitialized(const UGameFrameworkComponentManager* Manager, APawn* Pawn) const
{
	const ABasePlayerState* BasePS = GetPlayerState<ABasePlayerState>();

	const auto CurrentState = Manager->GetInitStateForFeature(Pawn, UPawnExtensionComponent::Name_ActorFeatureName);
	ULOG_INFO(LogTemp, "======== Current State: %s", *CurrentState.ToString());

	const bool PawnExtensionInitialized = Manager->HasFeatureReachedInitState(
		Pawn, UPawnExtensionComponent::Name_ActorFeatureName, InitStateTags::DATA_INITIALIZED);

	return BasePS && PawnExtensionInitialized;
}

bool UHeroComponent::CanTransitionToGameplayReady() const
{
	// TODO add ability initialization checks?
	return true;
}

void UHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
                                           FGameplayTag DesiredState)
{
	if (CurrentState != InitStateTags::DATA_AVAILABLE || DesiredState != InitStateTags::DATA_INITIALIZED) { return; }

	const APawn* Pawn = GetPawn<APawn>();
	ABasePlayerState* BasePS = GetPlayerState<ABasePlayerState>();
	if (!ensure(Pawn && BasePS)) { return; }

	const UGasPawnData* PawnData = nullptr;

	if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		PawnData = PawnExtComp->GetPawnData<UGasPawnData>();

		// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
		// The ability system component and attribute sets live on the player state.
		PawnExtComp->InitializeAbilitySystem(BasePS->GetBaseAbilitySystemComponent(), BasePS);
	}

	if (Pawn->InputComponent) { InitializePlayerInput(Pawn->InputComponent); }

	// Hook up the delegate for all pawns, in case we spectate later
	if (!PawnData) {}

	// 	if (ULyraCameraComponent* CameraComponent = ULyraCameraComponent::FindCameraComponent(Pawn))
	// 	{
	// 		CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
	// 	}
}

void UHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName != UPawnExtensionComponent::Name_ActorFeatureName || Params.FeatureState !=
		InitStateTags::DATA_INITIALIZED) { return; }

	// If the extension component says all other components are initialized,
	// try to progress to next state
	CheckDefaultInitialization();
}

void UHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = {
		InitStateTags::SPAWNED,
		InitStateTags::DATA_AVAILABLE,
		InitStateTags::DATA_INITIALIZED,
		InitStateTags::GAMEPLAY_READY
	};

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}
#pragma endregion

void UHeroComponent::OnRegister()
{
	Super::OnRegister();
	if (GetPawn<APawn>())
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
		return;
	}

	LOG_INFO(LogGAS,
	         "This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint.");

#if WITH_EDITOR
	if (GIsEditor)
	{
		static const FText Message = NSLOCTEXT("LyraHeroComponent", "NotOnPawnError",
		                                       "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
		static const FName HeroMessageLogName = TEXT("LyraHeroComponent");

		FMessageLog(HeroMessageLogName).Error()
		                               ->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
		                               ->AddToken(FTextToken::Create(Message));

		FMessageLog(HeroMessageLogName).Open();
	}
#endif
}

void UHeroComponent::BeginPlay()
{
	Super::BeginPlay();
	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UPawnExtensionComponent::Name_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(InitStateTags::SPAWNED));
	CheckDefaultInitialization();
}

void UHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}

void UHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) { return; }

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	check(LocalPlayer);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	RegisterInputMappings(Subsystem);
	BindInputActions(PlayerInputComponent, Subsystem);


	if (ensure(!bReadyToBindInputs)) { bReadyToBindInputs = true; }

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
		const_cast<APlayerController*>(PC), NAME_BIND_INPUTS_NOW);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(
		const_cast<APawn*>(Pawn), NAME_BIND_INPUTS_NOW);
}

void UHeroComponent::RegisterInputMappings(UEnhancedInputLocalPlayerSubsystem* Subsystem)
{
	Subsystem->ClearAllMappings();
	for (const auto& [InputMapping, Priority, bRegisterWithSettings] : DefaultInputMappings)
	{
		const UInputMappingContext* Imc = InputMapping.Get();
		if (!Imc)
		{
			LOG_ERROR(LogGAS, "Imc not found ");
			continue;
		}

		LOG_INFO(LogGAS, "%s found ", *Imc->GetName());

		if (!bRegisterWithSettings)
		{
			LOG_INFO(LogGAS, "%s found but will not be registered because bRegisterWithSettings is false",
			         *Imc->GetName());
			continue;
		}

		const auto Settings = Subsystem->GetUserSettings();
		if (!Settings)
		{
			LOG_INFO(LogGAS, "Settings not found ");
			continue;
		}

		Settings->RegisterInputMappingContext(Imc);

		FModifyContextOptions Options = {};
		Options.bIgnoreAllPressedKeysUntilRelease = false;
		// Actually add the config to the local player
		Subsystem->AddMappingContext(Imc, Priority, Options);
	}
}

void UHeroComponent::BindInputActions(UInputComponent* PlayerInputComponent,
                                      const UEnhancedInputLocalPlayerSubsystem* Subsystem)
{
	const auto Pawn = GetPawn<APawn>();
	if (!Pawn) { return; }

	const auto PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	if (!PawnExtComp) { return; }

	const auto PawnData = PawnExtComp->GetPawnData<UGasPawnData>();
	if (!PawnData) { return; }

	const auto InputConfig = PawnData->InputConfig;
	if (!InputConfig) { return; }

	const auto InputComponent = Cast<ULyraInputComponent>(PlayerInputComponent);
	if (!InputComponent)
	{
		LOG_ERROR(LogGAS, "Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. "
		          "Change the input component to ULyraInputComponent or a subclass of it.");
		return;
	}

	// Add the key mappings that may have been set by the player
	InputComponent->AddInputMappings(InputConfig, Subsystem);

	// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
	// be triggered directly by these input actions Triggered events.
	TArray<uint32> BindHandles;
	InputComponent->BindAbilityActionList(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,
	                                      &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

	InputComponent->BindNativeAction(InputConfig, InputTags::MOVE, ETriggerEvent::Triggered, this,
	                                 &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
	InputComponent->BindNativeAction(InputConfig, InputTags::LOOK_MOUSE, ETriggerEvent::Triggered, this,
	                                 &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
	InputComponent->BindNativeAction(InputConfig, InputTags::LOOK_STICK, ETriggerEvent::Triggered, this,
	                                 &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
	// InputComponent->BindNativeAction(InputConfig, InputTags::CROUCH, ETriggerEvent::Triggered, this,
	//                          &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
	InputComponent->BindNativeAction(InputConfig, InputTags::AUTORUN, ETriggerEvent::Triggered, this,
	                                 &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
}

void UHeroComponent::Input_AbilityInputTagPressed(const FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) { return; }

	const UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	if (!PawnExtComp) { return; }

	if (UBaseAbilitySystemComponent* Asc = PawnExtComp->GetBaseAbilitySystemComponent())
	{
		Asc->SetAbilityInputTagPressed(InputTag);
	}
}

void UHeroComponent::Input_AbilityInputTagReleased(const FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) { return; }

	const UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	if (!PawnExtComp) { return; }

	if (UBaseAbilitySystemComponent* Asc = PawnExtComp->GetBaseAbilitySystemComponent())
	{
		Asc->SetAbilityInputTagReleased(InputTag);
	}
}

void UHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	// todo implement this function later
	LOG_INFO(LogGAS, "--------------- Input_Move ----------");
}

void UHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	// todo implement this function later
	LOG_INFO(LogGAS, "--------------- Input_LookMouse ----------");
}

void UHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	// todo implement this function later
	LOG_INFO(LogGAS, "--------------- Input_LookStick ----------");
}

void UHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	// todo implement this function later
	LOG_INFO(LogGAS, "--------------- Input_Move ----------");
}

void UHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
	// todo impl
	LOG_INFO(LogGAS, "--------------- Input_AutoRun ----------");
}
