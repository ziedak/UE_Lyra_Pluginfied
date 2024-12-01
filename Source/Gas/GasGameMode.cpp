// Copyright Epic Games, Inc. All Rights Reserved.

#include "GasGameMode.h"
#include "GasCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGasGameMode::AGasGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
