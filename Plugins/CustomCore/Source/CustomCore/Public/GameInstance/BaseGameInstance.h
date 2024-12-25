// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonGameInstance.h"

#include "BaseGameInstance.generated.h"

enum class ECommonUserOnlineContext : uint8;
/**
 * 
 */
UCLASS(Abstract)
class CUSTOMCORE_API UBaseGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public :
	UBaseGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{
	}

	APlayerController* GetPrimaryPlayerController() const;
	virtual bool CanJoinRequestedSession() const override;

	virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken,
	                                            const FOnEncryptionKeyResponse& Delegate) override;
	virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

protected:
	virtual void Init() override;
	virtual void Shutdown() override;

	void OnPreClientTravelToSession(FString& URL);

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;
};
