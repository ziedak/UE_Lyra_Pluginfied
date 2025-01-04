// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "CameraModeView.generated.h"

/**
 * FLyraCameraModeView
 *
 *	View data produced by the camera mode that is used to blend camera modes.
 */
struct FCameraModeView
{
	FCameraModeView();

	void Blend(const FCameraModeView& Other, float OtherWeight);

	FVector Location;
	FRotator Rotation;
	FRotator ControlRotation;
	float FieldOfView;
};