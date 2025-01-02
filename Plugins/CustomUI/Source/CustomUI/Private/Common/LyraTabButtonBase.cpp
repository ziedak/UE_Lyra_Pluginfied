// Copyright Epic Games, Inc. All Rights Reserved.

#include "Common/LyraTabButtonBase.h"

#include "CommonLazyImage.h"
#include "Common/LyraTabListWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraTabButtonBase)

class UObject;
struct FSlateBrush;

void ULyraTabButtonBase::SetIconFromLazyObject(const TSoftObjectPtr<UObject>& LazyObject) const
{
	if (LazyImage_Icon) { LazyImage_Icon->SetBrushFromLazyDisplayAsset(LazyObject); }
}

void ULyraTabButtonBase::SetIconBrush(const FSlateBrush& Brush) const
{
	if (LazyImage_Icon) { LazyImage_Icon->SetBrush(Brush); }
}

void ULyraTabButtonBase::SetTabLabelInfo_Implementation(const FLyraTabDescriptor& TabLabelInfo)
{
	SetButtonText(TabLabelInfo.TabText);
	SetIconBrush(TabLabelInfo.IconBrush);
}
