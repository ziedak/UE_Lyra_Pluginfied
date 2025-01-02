// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Subsystem/UIManagerSubsystem.h"
#include "CommonLocalPlayer.h"
#include "GameUIPolicy.h"
#include "PrimaryGameLayout.h"
#include "GameFramework/HUD.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(UIManagerSubsystem)

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UUIManagerSubsystem::Tick),
	                                                  0.0f);
}

void UUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

bool UUIManagerSubsystem::Tick(float DeltaTime)
{
	SyncRootLayoutVisibilityToShowHUD();
	return true;
}

void UUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD()
{
	const UGameUIPolicy* UIPolicy = GetCurrentUIPolicy();
	if (!UIPolicy) return;

	for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
	{
		bool bShouldShowUI = true;

		if (const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
		{
			const AHUD* HUD = PC->GetHUD();
			if (HUD && !HUD->bShowHUD) bShouldShowUI = false;
		}
		UPrimaryGameLayout* RootLayout = UIPolicy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer));
		if (!RootLayout) continue;
		//TODO verify this 
		const ESlateVisibility DesiredVisibility = bShouldShowUI
			                                           ? ESlateVisibility::SelfHitTestInvisible
			                                           : ESlateVisibility::Collapsed;
		if (DesiredVisibility != RootLayout->GetVisibility()) RootLayout->SetVisibility(DesiredVisibility);
	}
}