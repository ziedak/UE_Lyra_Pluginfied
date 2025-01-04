// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraModes/CameraModeStack.h"
#include "CameraModes/CustomCameraMode.h"
#include "Engine/Canvas.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(CameraModeStack)

UCameraModeStack::UCameraModeStack() { bIsActive = true; }

void UCameraModeStack::ActivateStack()
{
	if (bIsActive) return;

	bIsActive = true;

	// Notify camera modes that they are being activated.
	for (UCustomCameraMode* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->OnActivation();
	}
}

void UCameraModeStack::DeactivateStack()
{
	if (!bIsActive) return;
	bIsActive = false;

	// Notify camera modes that they are being deactivated.
	for (UCustomCameraMode* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->OnDeactivation();
	}
}

void UCameraModeStack::PushCameraMode(const TSubclassOf<UCustomCameraMode>& CameraModeClass)
{
	if (!CameraModeClass) return;

	UCustomCameraMode* CameraMode = GetCameraModeInstance(CameraModeClass);
	check(CameraMode);

	int32 StackSize = CameraModeStack.Num();

	// Already top of stack.
	if (StackSize > 0 && CameraModeStack[0] == CameraMode) return;

	// See if it's already in the stack and remove it.
	// Figure out how much it was contributing to the stack.
	int32 ExistingStackIndex = INDEX_NONE;
	float ExistingStackContribution = 1.0f;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		if (CameraModeStack[StackIndex] == CameraMode)
		{
			ExistingStackIndex = StackIndex;
			ExistingStackContribution *= CameraMode->GetBlendWeight();
			break;
		}
		ExistingStackContribution *= 1.0f - CameraModeStack[StackIndex]->GetBlendWeight();
	}

	if (ExistingStackIndex != INDEX_NONE)
	{
		CameraModeStack.RemoveAt(ExistingStackIndex);
		StackSize--;
	}
	else ExistingStackContribution = 0.0f;

	// Decide what initial weight to start with.
	const bool bShouldBlend = CameraMode->GetBlendTime() > 0.0f && StackSize > 0;
	const float BlendWeight = bShouldBlend ? ExistingStackContribution : 1.0f;

	CameraMode->SetBlendWeight(BlendWeight);

	// Add new entry to top of stack.
	CameraModeStack.Insert(CameraMode, 0);

	// Make sure stack bottom is always weighted 100%.
	CameraModeStack.Last()->SetBlendWeight(1.0f);

	// Let the camera mode know if it's being added to the stack.
	if (ExistingStackIndex == INDEX_NONE) CameraMode->OnActivation();
}

bool UCameraModeStack::EvaluateStack(const float DeltaTime, FCameraModeView& OutCameraModeView)
{
	if (!bIsActive) return false;

	UpdateStack(DeltaTime);
	BlendStack(OutCameraModeView);

	return true;
}

UCustomCameraMode* UCameraModeStack::GetCameraModeInstance(const TSubclassOf<UCustomCameraMode>& CameraModeClass)
{
	check(CameraModeClass);

	// First see if we already created one.
	for (UCustomCameraMode* CameraMode : CameraModeInstances) { if (!CameraMode && CameraMode->GetClass() == CameraModeClass) return CameraMode; }

	// Not found, so we need to create it.
	UCustomCameraMode* NewCameraMode = NewObject<UCustomCameraMode>(GetOuter(), CameraModeClass, NAME_None, RF_NoFlags);
	check(NewCameraMode);

	CameraModeInstances.Add(NewCameraMode);

	return NewCameraMode;
}

void UCameraModeStack::UpdateStack(const float DeltaTime)
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0) return;

	int32 RemoveCount = 0;
	int32 RemoveIndex = INDEX_NONE;

	for (int32 StackIndex = 0; StackIndex < StackSize; ++StackIndex)
	{
		UCustomCameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->UpdateCameraMode(DeltaTime);

		if (CameraMode->GetBlendWeight() >= 1.0f)
		{
			// Everything below this mode is now irrelevant and can be removed.
			RemoveIndex = StackIndex + 1;
			RemoveCount = StackSize - RemoveIndex;
			break;
		}
	}

	if (RemoveCount <= 0) return;

	// Let the camera modes know they being removed from the stack.
	for (int32 StackIndex = RemoveIndex; StackIndex < StackSize; ++StackIndex)
	{
		UCustomCameraMode* CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		CameraMode->OnDeactivation();
	}

	CameraModeStack.RemoveAt(RemoveIndex, RemoveCount);
}

void UCameraModeStack::BlendStack(FCameraModeView& OutCameraModeView) const
{
	const int32 StackSize = CameraModeStack.Num();
	if (StackSize <= 0) return;

	// Start at the bottom and blend up the stack
	const UCustomCameraMode* CameraMode = CameraModeStack[StackSize - 1];
	check(CameraMode);

	OutCameraModeView = CameraMode->GetCameraModeView();

	for (int32 StackIndex = StackSize - 2; StackIndex >= 0; --StackIndex)
	{
		CameraMode = CameraModeStack[StackIndex];
		check(CameraMode);

		OutCameraModeView.Blend(CameraMode->GetCameraModeView(), CameraMode->GetBlendWeight());
	}
}

void UCameraModeStack::DrawDebug(UCanvas* Canvas) const
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString(TEXT("   --- Camera Modes (Begin) ---")));

	for (const UCustomCameraMode* CameraMode : CameraModeStack)
	{
		check(CameraMode);
		CameraMode->DrawDebug(Canvas);
	}

	DisplayDebugManager.SetDrawColor(FColor::Green);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("   --- Camera Modes (End) ---")));
}

void UCameraModeStack::GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const
{
	if (CameraModeStack.Num() == 0)
	{
		OutWeightOfTopLayer = 1.0f;
		OutTagOfTopLayer = FGameplayTag();
		return;
	}
	const UCustomCameraMode* TopEntry = CameraModeStack.Last();
	check(TopEntry);
	OutWeightOfTopLayer = TopEntry->GetBlendWeight();
	OutTagOfTopLayer = TopEntry->GetCameraTypeTag();
}