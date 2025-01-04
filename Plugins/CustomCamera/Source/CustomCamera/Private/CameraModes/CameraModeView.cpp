// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraModes/CameraModeView.h"

#include "Global/CustomCamera.h"

FCameraModeView::FCameraModeView()
	: Location(ForceInit)
	  , Rotation(ForceInit)
	  , ControlRotation(ForceInit)
	  , FieldOfView(CAMERA_DEFAULT_FOV) {}

void FCameraModeView::Blend(const FCameraModeView& Other, const float OtherWeight)
{
	if (OtherWeight <= 0.0f) return;
	if (OtherWeight >= 1.0f)
	{
		*this = Other;
		return;
	}

	Location = FMath::Lerp(Location, Other.Location, OtherWeight);

	const FRotator DeltaRotation = (Other.Rotation - Rotation).GetNormalized();
	Rotation = Rotation + (OtherWeight * DeltaRotation);

	const FRotator DeltaControlRotation = (Other.ControlRotation - ControlRotation).GetNormalized();
	ControlRotation = ControlRotation + (OtherWeight * DeltaControlRotation);

	FieldOfView = FMath::Lerp(FieldOfView, Other.FieldOfView, OtherWeight);
}
