// Fill out your copyright notice in the Description page of Project Settings.


#include "Cheat/BaseCheatManager.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Cheat/CheatConsole.h"
#include "Cheat/CheatDeveloperSettings.h"
#include "Cheat/CheatTags.h"
#include "Cheat/CheatToRun.h"
#include "Cheat/Interfaces/CheatServer.h"
#include "Engine/Console.h"
#include "Engine/DebugCameraController.h"
#include "GameFramework/HUD.h"
#include "Log/Log.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseCheatManager)


UBaseCheatManager::UBaseCheatManager()
{
	DebugCameraControllerClass = ADebugCameraController::StaticClass();
}

void UBaseCheatManager::InitCheatManager()
{
	Super::InitCheatManager();

#if WITH_EDITOR
	if (GIsEditor)
	{
		APlayerController* PC = GetOuterAPlayerController();
		for (const FCheatToRun& CheatRow : GetDefault<UCheatDeveloperSettings>()->CheatsToRun)
		{
			if (CheatRow.Phase == ECheatExecutionTime::OnCheatManagerCreated)
				PC->ConsoleCommand(CheatRow.Cheat, /*bWriteToLog=*/ true);
		}
	}
#endif

	if (CheatConsole::GSTART_IN_GOD_MODE)
		God();
}

void UBaseCheatManager::CheatOutputText(const FString& TextToOutput)
{
#if !UE_BUILD_SHIPPING
	// Output to the console.
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportConsole)
		GEngine->GameViewport->ViewportConsole->OutputText(TextToOutput);

	// Output to log.
	UE_LOG(LogCheat, Display, TEXT("%s"), *TextToOutput);
#endif
}

void UBaseCheatManager::Cheat(const FString& Msg) const
{
	if (const auto CheatServer = Cast<ICheatServer>(GetOuterAPlayerController()))
		CheatServer->ServerCheat(Msg.Left(128));
}

void UBaseCheatManager::CheatAll(const FString& Msg) const
{
	if (const auto CheatServer = Cast<ICheatServer>(GetOuterAPlayerController()))
		CheatServer->ServerCheatAll(Msg.Left(128));
}

void UBaseCheatManager::PlayNextGame()
{
	// 	USystemStatics::PlayNextGame(this);

	UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
		return;

	const FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
	FURL LastURL = WorldContext.LastURL;

#if WITH_EDITOR
	// To transition during PIE we need to strip the PIE prefix from maps.
	LastURL.Map = UWorld::StripPIEPrefixFromPackageName(LastURL.Map, WorldContext.World()->StreamingLevelsPrefix);
#endif

	// Add seamless travel option as we want to keep clients connected. This will fall back to hard travel if seamless is disabled
	LastURL.AddOption(TEXT("SeamlessTravel"));

	FString URL = LastURL.ToString();
	// If we don't remove the host/port info the server travel will fail.
	URL.RemoveFromStart(LastURL.GetHostPortString());

	constexpr auto bAbsolute = false; // we want to use TRAVEL_Relative
	constexpr auto bShouldSkipGameNotify = false;
	World->ServerTravel(URL, bAbsolute, bShouldSkipGameNotify);
}

void UBaseCheatManager::EnableDebugCamera()
{
	Super::EnableDebugCamera();
}

void UBaseCheatManager::DisableDebugCamera()
{
	FVector DebugCameraLocation;
	FRotator DebugCameraRotation;

	const ADebugCameraController* DebugCC = Cast<ADebugCameraController>(GetOuter());
	APlayerController* OriginalPC = nullptr;

	if (DebugCC)
	{
		OriginalPC = DebugCC->OriginalControllerRef;
		DebugCC->GetPlayerViewPoint(DebugCameraLocation, DebugCameraRotation);
	}

	Super::DisableDebugCamera();

	if (OriginalPC && OriginalPC->PlayerCameraManager && OriginalPC->PlayerCameraManager->CameraStyle == CheatConsole::GName_Fixed)
	{
		OriginalPC->SetInitialLocationAndRotation(DebugCameraLocation, DebugCameraRotation);

		OriginalPC->PlayerCameraManager->ViewTarget.POV.Location = DebugCameraLocation;
		OriginalPC->PlayerCameraManager->ViewTarget.POV.Rotation = DebugCameraRotation;
		OriginalPC->PlayerCameraManager->PendingViewTarget.POV.Location = DebugCameraLocation;
		OriginalPC->PlayerCameraManager->PendingViewTarget.POV.Rotation = DebugCameraRotation;
	}
}

bool UBaseCheatManager::InDebugCamera() const
{
	return (Cast<ADebugCameraController>(GetOuter()) ? true : false);
}

void UBaseCheatManager::EnableFixedCamera()
{
	const auto DebugCC = Cast<ADebugCameraController>(GetOuter());
	const auto PC = DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController();

	if (PC && PC->PlayerCameraManager)
		PC->SetCameraMode(CheatConsole::GName_Fixed);
}

void UBaseCheatManager::DisableFixedCamera()
{
	const auto DebugCC = Cast<ADebugCameraController>(GetOuter());
	const auto PC = DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController();

	if (PC && PC->PlayerCameraManager)
		PC->SetCameraMode(NAME_Default);
}

bool UBaseCheatManager::InFixedCamera() const
{
	const auto DebugCC = Cast<ADebugCameraController>(GetOuter());
	const auto PC = (DebugCC ? ToRawPtr(DebugCC->OriginalControllerRef) : GetOuterAPlayerController());

	if (PC && PC->PlayerCameraManager)
		return (PC->PlayerCameraManager->CameraStyle == CheatConsole::GName_Fixed);

	return false;
}

void UBaseCheatManager::ToggleFixedCamera()
{
	InFixedCamera() ? DisableFixedCamera() : EnableFixedCamera();
}

void UBaseCheatManager::CycleDebugCameras()
{
	if (!CheatConsole::GEnable_Debug_Camera_Cycling)
		return;

	if (InDebugCamera())
	{
		EnableFixedCamera();
		DisableDebugCamera();
	}
	else if (InFixedCamera())
	{
		DisableFixedCamera();
		DisableDebugCamera();
	}
	else
	{
		EnableDebugCamera();
		DisableFixedCamera();
	}
}

void UBaseCheatManager::CycleAbilitySystemDebug()
{
	const auto PC = GetOuterAPlayerController();

	if (!PC || !PC->GetHUD())
		return;
	if (!PC->GetHUD()->bShowDebugInfo || !PC->GetHUD()->DebugDisplay.Contains(TEXT("AbilitySystem")))
		PC->GetHUD()->ShowDebug(TEXT("AbilitySystem"));

	PC->ConsoleCommand(TEXT("AbilitySystem.Debug.NextCategory"));
}

void UBaseCheatManager::CancelActivatedAbilities()
{
	const auto CheatServer = GetICheatServer();
	if (!CheatServer)
		return;

	constexpr auto bReplicateCancelAbility = true;
	CheatServer->CancelInputActivatedAbilities(bReplicateCancelAbility);
}


void UBaseCheatManager::AddTagToSelf(const FString TagName)
{
	FGameplayTag Tag = FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (const auto CheatServer = GetICheatServer())
			CheatServer->AddDynamicTagGameplayEffect(Tag);
		return;
	}
	UE_LOG(LogCheat, Display, TEXT("AddTagToSelf: Could not find any tag matching [%s]."), *TagName);
}

void UBaseCheatManager::RemoveTagFromSelf(const FString TagName)
{
	FGameplayTag Tag = FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (const auto CheatServer = GetICheatServer())
			CheatServer->RemoveDynamicTagGameplayEffect(Tag);
		return;
	}

	UE_LOG(LogCheat, Display, TEXT("RemoveTagFromSelf: Could not find any tag matching [%s]."), *TagName);
}

void UBaseCheatManager::DamageSelf(const float DamageAmount)
{
	if (const auto ICheat = GetICheatServer())
	{
		if (const auto Asc = ICheat->GetAbilitySystemComponent())
			ApplySetByCallerDamage(Asc, DamageAmount);
	}
}

void UBaseCheatManager::DamageTarget(const float DamageAmount)
{
	const auto Pc = Cast<APlayerController>(GetOuterAPlayerController());
	if (!Pc)
		return;

	const auto CheatServer = Cast<ICheatServer>(Pc);
	if (!CheatServer)
		return;

	if (Pc->GetNetMode() == NM_Client)
	{
		// Automatically send cheat to server for convenience.
		CheatServer->ServerCheat(FString::Printf(TEXT("DamageTarget %.2f"), DamageAmount));
		return;
	}

	FHitResult TargetHitResult;
	const AActor* TargetActor = GetTarget(Pc, TargetHitResult);

	if (UAbilitySystemComponent* TargetAsc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
		ApplySetByCallerDamage(TargetAsc, DamageAmount);
}

void UBaseCheatManager::ApplySetByCallerDamage(UAbilitySystemComponent* Asc, const float DamageAmount) const
{
	check(Asc);
	if (const auto ICheat = GetICheatServer())
		ICheat->ApplySetByCallerDamage(Asc, SetByCallerTags::DAMAGE, DamageAmount);
}

void UBaseCheatManager::HealSelf(const float HealAmount)
{
	if (const auto ICheat = GetICheatServer())
	{
		if (const auto Asc = ICheat->GetAbilitySystemComponent())
			ApplySetByCallerHeal(Asc, HealAmount);
	}
}

void UBaseCheatManager::HealTarget(const float HealAmount)
{
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		FHitResult TargetHitResult;
		const AActor* TargetActor = GetTarget(PC, TargetHitResult);
		if (UAbilitySystemComponent* TargetAsc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
			ApplySetByCallerHeal(TargetAsc, HealAmount);
	}
}

void UBaseCheatManager::ApplySetByCallerHeal(UAbilitySystemComponent* Asc, const float HealAmount) const
{
	check(Asc);
	if (const auto ICheat = GetICheatServer())
		ICheat->ApplySetByCallerHeal(Asc, SetByCallerTags::HEAL, HealAmount);
}

ICheatServer* UBaseCheatManager::GetICheatServer() const
{
	const auto Controller = GetOuterAPlayerController();
	if (!Controller)
	{
		UE_LOG(LogCheat, Warning, TEXT("God: No player controller found."));
		return nullptr;
	}
	const auto ICheat = Cast<ICheatServer>(Controller);
	if (!ICheat)
	{
		UE_LOG(LogCheat, Warning, TEXT("God: The controller doesn't implement ICheatServer interface."));
		return nullptr;
	}
	return ICheat;
}


void UBaseCheatManager::DamageSelfDestruct()
{
	const auto CheatServer = GetICheatServer();
	if (!CheatServer)
		return;
	CheatServer->DamageSelfDestruct();
}

void UBaseCheatManager::God()
{
	const auto Controller = GetOuterAPlayerController();
	if (!Controller)
	{
		UE_LOG(LogCheat, Warning, TEXT("God: No player controller found."));
		return;
	}
	const auto CheatServer = Cast<ICheatServer>(Controller);
	if (!CheatServer)
		return;

	if (Controller->GetNetMode() == NM_Client)
	{
		// Automatically send cheat to server for convenience.
		CheatServer->ServerCheat(FString::Printf(TEXT("God")));
		return;
	}

	CheatServer->ToggleDynamicTagGameplayEffect(CheatTags::GOD_MODE);
}

// if (APlayerController* PC = Cast<APlayerController>(GetOuterAPlayerController()))
// {
// 	if (PC->GetNetMode() == NM_Client)
// 	{
// 		// Automatically send cheat to server for convenience.
// 		PC->ServerCheat(FString::Printf(TEXT("God")));
// 		return;
// 	}
//
// 	if (UAbilitySystemComponent* Asc = PC->GetAbilitySystemComponent())
// 	{
// 		const FGameplayTag Tag = GameplayTags::Cheat_GodMode;
// 		const bool bHasTag = Asc->HasMatchingGameplayTag(Tag);
//
// 		if (bHasTag)
// 			Asc->RemoveDynamicTagGameplayEffect(Tag);
// 		else
// 			Asc->AddDynamicTagGameplayEffect(Tag);
// 	}
// }
// }

void UBaseCheatManager::UnlimitedHealth()
{
	const auto CheatServer = GetICheatServer();
	if (!CheatServer)
		return;
	CheatServer->ToggleDynamicTagGameplayEffect(CheatTags::UNLIMITED_HEALTH);
	// if (UAbilitySystemComponent* Asc = GetICheatServer())
	// {
	// 	const FGameplayTag Tag = UNLIMITED_HEALTH;
	// 	const bool bHasTag = Asc->HasMatchingGameplayTag(Tag);
	//
	// 	if (Enabled && !bHasTag)
	// 	{
	// 		Asc->AddDynamicTagGameplayEffect(Tag);
	// 	}
	// 	else if (!Enabled && bHasTag)
	// 	{
	// 		Asc->RemoveDynamicTagGameplayEffect(Tag);
	// 	}
	// }
}
