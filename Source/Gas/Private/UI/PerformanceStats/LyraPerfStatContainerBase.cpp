// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/PerformanceStats/LyraPerfStatContainerBase.h"
#include "Blueprint/WidgetTree.h"
#include "Settings/LyraSettingsLocal.h"
#include "UI/PerformanceStats/LyraPerfStatWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPerfStatContainerBase)

//////////////////////////////////////////////////////////////////////
// ULyraPerfStatsContainerBase

ULyraPerfStatContainerBase::ULyraPerfStatContainerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

void ULyraPerfStatContainerBase::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateVisibilityOfChildren();

	ULyraSettingsLocal::Get()->OnPerfStatDisplayStateChanged().AddUObject(this, &ThisClass::UpdateVisibilityOfChildren);
}

void ULyraPerfStatContainerBase::NativeDestruct()
{
	ULyraSettingsLocal::Get()->OnPerfStatDisplayStateChanged().RemoveAll(this);

	Super::NativeDestruct();
}

void ULyraPerfStatContainerBase::UpdateVisibilityOfChildren()
{
	const ULyraSettingsLocal* UserSettings = ULyraSettingsLocal::Get();

	const bool bShowTextWidgets = StatDisplayModeFilter == EStatDisplayMode::TextOnly || StatDisplayModeFilter == EStatDisplayMode::TextAndGraph;
	const bool bShowGraphWidgets = StatDisplayModeFilter == EStatDisplayMode::GraphOnly || StatDisplayModeFilter == EStatDisplayMode::TextAndGraph;

	check(WidgetTree);
	WidgetTree->ForEachWidget([&](UWidget* Widget){
		if (ULyraPerfStatWidgetBase* TypedWidget = Cast<ULyraPerfStatWidgetBase>(Widget))
		{
			const EStatDisplayMode SettingMode = UserSettings->GetPerfStatDisplayState(
				TypedWidget->GetStatToDisplay());

			bool bShowWidget = false;
			switch (SettingMode)
			{
			case EStatDisplayMode::Hidden: bShowWidget = false;
				break;
			case EStatDisplayMode::TextOnly: bShowWidget = bShowTextWidgets;
				break;
			case EStatDisplayMode::GraphOnly: bShowWidget = bShowGraphWidgets;
				break;
			case EStatDisplayMode::TextAndGraph: bShowWidget = bShowTextWidgets || bShowGraphWidgets;
				break;
			}

			TypedWidget->SetVisibility(bShowWidget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	});
}