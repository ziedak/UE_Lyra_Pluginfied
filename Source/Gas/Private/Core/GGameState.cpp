// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GGameState.h"
#include "Player/BasePlayerState.h"


void AGGameState::SeamlessTravelTransitionCheckpoint(bool bToTransitionMap)
{
	// Remove inactive and bots
	for (int32 i = PlayerArray.Num() - 1; i >= 0; i--)
	{
		const auto PlayerState = Cast<ABasePlayerState>(PlayerArray[i]);
		if (PlayerState && (PlayerState->IsABot() || PlayerState->IsInactive())) { RemovePlayerState(PlayerState); }
	}
}
