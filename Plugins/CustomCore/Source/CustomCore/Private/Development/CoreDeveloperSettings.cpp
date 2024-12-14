// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/CoreDeveloperSettings.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CoreDeveloperSettings)

#define LOCTEXT_NAMESPACE "CoreCheats"


FName UCoreDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

#if WITH_EDITOR
void UCoreDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplySettings();
}

void UCoreDeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);
	ApplySettings();
}

void UCoreDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();
	ApplySettings();
}

void UCoreDeveloperSettings::ApplySettings()
{
}

void UCoreDeveloperSettings::OnPlayInEditorStarted() const
{
	// Show a notification toast to remind the user that there's an experience override set
	if (ExperienceOverride.IsValid())
	{
		FNotificationInfo Info(FText::Format(
			LOCTEXT("ExperienceOverrideActive", "Developer Settings Override\nExperience {0}"),
			FText::FromName(ExperienceOverride.PrimaryAssetName)
			));
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
}
#endif

#undef LOCTEXT_NAMESPACE
