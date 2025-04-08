// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomSettings/EditConditions/WhenAutoQualitySupported.h"
#include "Performance/LyraPerformanceSettings.h"
#include "Interfaces/IPlayerSharedSettingsInterface.h"
#include "Engine/LocalPlayer.h"
#include "Settings/LyraSettingsLocal.h"

TSharedRef<FWhenAutoQualitySupported> FWhenAutoQualitySupported::KillIfNot()
{
	static TSharedRef<FWhenAutoQualitySupported> Instance = MakeShared<FWhenAutoQualitySupported>();
	return Instance;
}

void FWhenAutoQualitySupported::GatherEditState(const ULocalPlayer* InLocalPlayer,
                                                FGameSettingEditableState& InOutEditState) const
{
	const ULyraPlatformSpecificRenderingSettings* PlatformSettings = ULyraPlatformSpecificRenderingSettings::Get();
	const bool bCanUseDueToMobile = PlatformSettings->FramePacingMode == EFramePacingMode::MobileStyle;
	bool bCanBenchmark = false;
	if (const auto ISharedSettings = CastChecked<IPlayerSharedSettingsInterface>(InLocalPlayer)) bCanBenchmark = ISharedSettings->GetLocalSettings()->CanRunAutoBenchmark();

	if (!bCanUseDueToMobile && !bCanBenchmark) InOutEditState.Kill(TEXT("Auto quality not supported"));
}
