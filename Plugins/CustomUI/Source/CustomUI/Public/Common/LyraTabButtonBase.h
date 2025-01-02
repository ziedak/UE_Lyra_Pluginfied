// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraTabListWidgetBase.h"
#include "Foundation/LyraButtonBase.h"

#include "LyraTabButtonBase.generated.h"

class UCommonLazyImage;
class UObject;
struct FFrame;
struct FSlateBrush;

UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class CUSTOMUI_API ULyraTabButtonBase : public ULyraButtonBase, public ILyraTabButtonInterface
{
	GENERATED_BODY()

public:
	void SetIconFromLazyObject(const TSoftObjectPtr<UObject>& LazyObject) const;
	void SetIconBrush(const FSlateBrush& Brush) const;

protected:
	UFUNCTION()
	virtual void SetTabLabelInfo_Implementation(const FLyraTabDescriptor& TabLabelInfo) override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonLazyImage> LazyImage_Icon;
};
