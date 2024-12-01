// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BaseWorldSettings.h"

#include "EngineUtils.h"
#include "Engine/AssetManager.h"
#include "GameFramework/PlayerStart.h"
#include "Log/Loggger.h"
#include "Misc/UObjectToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseWorldSettings)


FPrimaryAssetId ABaseWorldSettings::GetDefaultGameplayExperience() const
{
	FPrimaryAssetId Result;
	if (DefaultGameplayExperience.IsNull())
		return Result;

	Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());

	if (!Result.IsValid())
	{
		UE_LOG(LogExperience, Error,
		       TEXT(
			       "%s.DefaultGameplayExperience is %s but that failed to resolve into an asset ID (you might need to add a path to the Asset Rules in your game feature plugin or project settings"
		       ),
		       *GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
	}

	return Result;
}

#if WITH_EDITOR
void ABaseWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	FMessageLog MapCheck("MapCheck");

	for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
	{
		const APlayerStart* PlayerStart = *PlayerStartIt;
		if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
		{
			MapCheck.Warning()
			        ->AddToken(FUObjectToken::Create(PlayerStart))
			        ->AddToken(FTextToken::Create(
				        FText::FromString("is a normal APlayerStart, replace with ABasePlayerStart.")));
		}
	}

	//@TODO: Make sure the soft object path is something that can actually be turned into a primary asset ID
	//(e.g., is not pointing to an experience in an unscanned directory)
}
#endif
