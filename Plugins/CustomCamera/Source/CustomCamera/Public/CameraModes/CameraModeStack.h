// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Object.h"
#include "CameraModeStack.generated.h"


class UCustomCameraMode;
struct FGameplayTag;
struct FCameraModeView;
/**
 * UCameraModeStack
 *
 *	Stack used for blending camera modes.
 */
UCLASS()
class UCameraModeStack : public UObject
{
	GENERATED_BODY()

public:
	UCameraModeStack();

	void ActivateStack();
	void DeactivateStack();

	bool IsStackActivate() const { return bIsActive; }

	void PushCameraMode(const TSubclassOf<UCustomCameraMode>& CameraModeClass);

	bool EvaluateStack(float DeltaTime, FCameraModeView& OutCameraModeView);

	void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

protected:
	UCustomCameraMode* GetCameraModeInstance(const TSubclassOf<UCustomCameraMode>& CameraModeClass);

	void UpdateStack(float DeltaTime);
	void BlendStack(FCameraModeView& OutCameraModeView) const;

	bool bIsActive;

	UPROPERTY()
	TArray<TObjectPtr<UCustomCameraMode>> CameraModeInstances;

	UPROPERTY()
	TArray<TObjectPtr<UCustomCameraMode>> CameraModeStack;
};