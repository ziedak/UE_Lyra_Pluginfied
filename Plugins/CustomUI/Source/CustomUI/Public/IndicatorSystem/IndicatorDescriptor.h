// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/SceneComponent.h"
#include "Types/SlateEnums.h"

#include "IndicatorDescriptor.generated.h"


class SWidget;
class UIndicatorDescriptor;
class ULyraIndicatorManagerComponent;
class UUserWidget;
struct FFrame;
struct FSceneViewProjectionData;

struct FIndicatorProjection
{
	bool Project(const UIndicatorDescriptor& IndicatorDescriptor, const FSceneViewProjectionData& InProjectionData,
	             const FVector2f& ScreenSize, FVector& OutScreenPositionWithDepth) const;
	TOptional<FVector> GetWorldLocation(const UIndicatorDescriptor& IndicatorDescriptor,
	                                    const USceneComponent* Component) const;
	bool ProjectComponentPoint(const UIndicatorDescriptor& IndicatorDescriptor,
	                           const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize,
	                           const FVector& ProjectWorldLocation, FVector& OutScreenPositionWithDepth) const;
	bool ProjectBoundingBox(const UIndicatorDescriptor& IndicatorDescriptor,
	                        const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize,
	                        const FVector& ProjectWorldLocation, EActorCanvasProjectionMode ProjectionMode,
	                        FVector& OutScreenPositionWithDepth) const;
	bool ProjectScreenBoundingBox(const UIndicatorDescriptor& IndicatorDescriptor,
	                              const FSceneViewProjectionData& InProjectionData, const FVector2f& ScreenSize,
	                              const FVector& ProjectWorldLocation, EActorCanvasProjectionMode ProjectionMode,
	                              FVector& OutScreenPositionWithDepth) const;
};

UENUM(BlueprintType)
enum class EActorCanvasProjectionMode : uint8
{
	ComponentPoint,
	ComponentBoundingBox,
	ComponentScreenBoundingBox,
	ActorBoundingBox,
	ActorScreenBoundingBox
};

/**
 * Describes and controls an active indicator.  It is highly recommended that your widget implements
 * IActorIndicatorWidget so that it can 'bind' to the associated data.
 */
UCLASS(BlueprintType)
class CUSTOMUI_API UIndicatorDescriptor : public UObject
{
	GENERATED_BODY()

public:
	UIndicatorDescriptor() {}

	UFUNCTION(BlueprintCallable)
	UObject* GetDataObject() const { return DataObject; }

	UFUNCTION(BlueprintCallable)
	void SetDataObject(UObject* InDataObject) { DataObject = InDataObject; }

	UFUNCTION(BlueprintCallable)
	USceneComponent* GetSceneComponent() const { return Component; }

	UFUNCTION(BlueprintCallable)
	void SetSceneComponent(USceneComponent* InComponent) { Component = InComponent; }

	UFUNCTION(BlueprintCallable)
	FName GetComponentSocketName() const { return ComponentSocketName; }

	UFUNCTION(BlueprintCallable)
	void SetComponentSocketName(const FName SocketName) { ComponentSocketName = SocketName; }

	UFUNCTION(BlueprintCallable)
	TSoftClassPtr<UUserWidget> GetIndicatorClass() const { return IndicatorWidgetClass; }

	UFUNCTION(BlueprintCallable)
	void SetIndicatorClass(const TSoftClassPtr<UUserWidget> InIndicatorWidgetClass)
	{
		IndicatorWidgetClass = InIndicatorWidgetClass;
	}

	// TODO Organize this better.
	TWeakObjectPtr<UUserWidget> IndicatorWidget;

	UFUNCTION(BlueprintCallable)
	void SetAutoRemoveWhenIndicatorComponentIsNull(const bool CanAutomaticallyRemove)
	{
		bAutoRemoveWhenIndicatorComponentIsNull = CanAutomaticallyRemove;
	}

	UFUNCTION(BlueprintCallable)
	bool GetAutoRemoveWhenIndicatorComponentIsNull() const { return bAutoRemoveWhenIndicatorComponentIsNull; }

	bool CanAutomaticallyRemove() const
	{
		return bAutoRemoveWhenIndicatorComponentIsNull && !IsValid(GetSceneComponent());
	}

	// Layout Properties
	//=======================

	UFUNCTION(BlueprintCallable)
	bool GetIsVisible() const { return IsValid(GetSceneComponent()) && bVisible; }

	UFUNCTION(BlueprintCallable)
	void SetDesiredVisibility(const bool InVisible) { bVisible = InVisible; }

	UFUNCTION(BlueprintCallable)
	EActorCanvasProjectionMode GetProjectionMode() const { return ProjectionMode; }

	UFUNCTION(BlueprintCallable)
	void SetProjectionMode(const EActorCanvasProjectionMode InProjectionMode) { ProjectionMode = InProjectionMode; }

	// Horizontal alignment to the point in space to place the indicator at.
	UFUNCTION(BlueprintCallable)
	EHorizontalAlignment GetHAlign() const { return HAlignment; }

	UFUNCTION(BlueprintCallable)
	void SetHAlign(const EHorizontalAlignment InHAlignment) { HAlignment = InHAlignment; }

	// Vertical alignment to the point in space to place the indicator at.
	UFUNCTION(BlueprintCallable)
	EVerticalAlignment GetVAlign() const { return VAlignment; }

	UFUNCTION(BlueprintCallable)
	void SetVAlign(const EVerticalAlignment InVAlignment) { VAlignment = InVAlignment; }

	// Clamp the indicator to the edge of the screen?
	UFUNCTION(BlueprintCallable)
	bool GetClampToScreen() const { return bClampToScreen; }

	UFUNCTION(BlueprintCallable)
	void SetClampToScreen(const bool bValue) { bClampToScreen = bValue; }

	// Show the arrow if clamping to the edge of the screen?
	UFUNCTION(BlueprintCallable)
	bool GetShowClampToScreenArrow() const { return bShowClampToScreenArrow; }

	UFUNCTION(BlueprintCallable)
	void SetShowClampToScreenArrow(const bool bValue) { bShowClampToScreenArrow = bValue; }

	// The position offset for the indicator in world space.
	UFUNCTION(BlueprintCallable)
	FVector GetWorldPositionOffset() const { return WorldPositionOffset; }

	UFUNCTION(BlueprintCallable)
	void SetWorldPositionOffset(const FVector Offset) { WorldPositionOffset = Offset; }

	// The position offset for the indicator in screen space.
	UFUNCTION(BlueprintCallable)
	FVector2D GetScreenSpaceOffset() const { return ScreenSpaceOffset; }

	UFUNCTION(BlueprintCallable)
	void SetScreenSpaceOffset(const FVector2D Offset) { ScreenSpaceOffset = Offset; }

	UFUNCTION(BlueprintCallable)
	FVector GetBoundingBoxAnchor() const { return BoundingBoxAnchor; }

	UFUNCTION(BlueprintCallable)
	void SetBoundingBoxAnchor(const FVector InBoundingBoxAnchor) { BoundingBoxAnchor = InBoundingBoxAnchor; }

	// Sorting Properties
	//=======================

	// Allows sorting the indicators (after they are sorted by depth), to allow some group of indicators
	// to always be in front of others.
	UFUNCTION(BlueprintCallable)
	int32 GetPriority() const { return Priority; }

	UFUNCTION(BlueprintCallable)
	void SetPriority(const int32 InPriority) { Priority = InPriority; }

	ULyraIndicatorManagerComponent* GetIndicatorManagerComponent() const { return ManagerPtr.Get(); }
	void SetIndicatorManagerComponent(ULyraIndicatorManagerComponent* InManager);

	UFUNCTION(BlueprintCallable)
	void UnregisterIndicator();

private:
	UPROPERTY()
	bool bVisible = true;
	UPROPERTY()
	bool bClampToScreen = false;
	UPROPERTY()
	bool bShowClampToScreenArrow = false;
	UPROPERTY()
	bool bOverrideScreenPosition = false;
	UPROPERTY()
	bool bAutoRemoveWhenIndicatorComponentIsNull = false;

	UPROPERTY()
	EActorCanvasProjectionMode ProjectionMode = EActorCanvasProjectionMode::ComponentPoint;

	UPROPERTY()
	TEnumAsByte<EHorizontalAlignment> HAlignment = HAlign_Center;

	UPROPERTY()
	TEnumAsByte<EVerticalAlignment> VAlignment = VAlign_Center;

	UPROPERTY()
	int32 Priority = 0;

	UPROPERTY()
	FVector BoundingBoxAnchor = FVector(0.5);

	UPROPERTY()
	FVector2D ScreenSpaceOffset = FVector2D::Zero();

	UPROPERTY()
	FVector WorldPositionOffset = FVector::Zero();

	friend class SActorCanvas;

	UPROPERTY()
	TObjectPtr<UObject> DataObject;

	UPROPERTY()
	TObjectPtr<USceneComponent> Component;

	UPROPERTY()
	FName ComponentSocketName = NAME_None;

	UPROPERTY()
	TSoftClassPtr<UUserWidget> IndicatorWidgetClass;

	UPROPERTY()
	TWeakObjectPtr<ULyraIndicatorManagerComponent> ManagerPtr;

	TWeakPtr<SWidget> Content;
	TWeakPtr<SWidget> CanvasHost;
};
