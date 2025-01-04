// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "GameUIManagerSubsystem.h"

#include "LyraUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UObject;

UCLASS()
class ULyraUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool Tick(float DeltaTime) const;
	bool ShouldShowUI(const ULocalPlayer* LocalPlayer) const;
	void SyncRootLayoutVisibilityToShowHUD() const;

	FTSTicker::FDelegateHandle TickHandle;
};