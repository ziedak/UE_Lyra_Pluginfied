// Copyright Epic Games, Inc. All Rights Reserved.

#include "Subsystem/LyraUIManagerSubsystem.h"

#include "CommonLocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/HUD.h"
#include "GameUIPolicy.h"
#include "PrimaryGameLayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraUIManagerSubsystem)

class FSubsystemCollectionBase;

void ULyraUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ULyraUIManagerSubsystem::Tick), 0.0f);
}

void ULyraUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

bool ULyraUIManagerSubsystem::Tick(float DeltaTime) const
{
	SyncRootLayoutVisibilityToShowHUD();
	return true;
}

bool ULyraUIManagerSubsystem::ShouldShowUI(const ULocalPlayer* LocalPlayer) const
{
	if (!LocalPlayer) return false;
	const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (!PC) return false;

	const auto HUD = PC->GetHUD();
	return HUD && !HUD->bShowHUD;
}

void ULyraUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD() const
{
	const UGameUIPolicy* Policy = GetCurrentUIPolicy();
	if (!Policy) return;

	for (const auto LocalPlayer : GetGameInstance()->GetLocalPlayers())
	{
		if (!LocalPlayer) return;

		const auto RootLayout = Policy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer));
		if (!RootLayout) continue;

		const ESlateVisibility DesiredVisibility = ShouldShowUI(LocalPlayer)
			                                           ? ESlateVisibility::SelfHitTestInvisible
			                                           : ESlateVisibility::Collapsed;
		if (DesiredVisibility == RootLayout->GetVisibility()) continue;

		RootLayout->SetVisibility(DesiredVisibility);
	}
}