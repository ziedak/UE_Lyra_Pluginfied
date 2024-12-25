// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstance/BaseGameInstance.h"
#include "GGameInstance.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class GAS_API UGGameInstance : public UBaseGameInstance
{
	GENERATED_BODY()

public:
	UGGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

protected:
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error,
	                                    ECommonUserPrivilege RequestedPrivilege,
	                                    ECommonUserOnlineContext OnlineContext) override;
};
