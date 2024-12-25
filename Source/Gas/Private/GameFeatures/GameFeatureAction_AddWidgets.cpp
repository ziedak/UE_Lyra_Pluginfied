#include "GameFeatures/GameFeatureAction_AddWidgets.h"

#include "Components/GameFrameworkComponentManager.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "GameplayTagContainer.h"
#include "UI/BaseHud.h"
#include "UIExtensionSystem.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddWidgets)


#define LOCTEXT_NAMESPACE "GameFeatures"

void UGameFeatureAction_AddWidgets::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FPerContextData* ActiveData = ContextData.Find(Context);
	if ensure(ActiveData) { Reset(*ActiveData); }
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddWidgets::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	for (const auto& [WidgetClass, SlotID] : Widgets)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient,
		                               WidgetClass.ToSoftObjectPath().GetAssetPath());
	}
}
#endif


void UGameFeatureAction_AddWidgets::AddToWorld(const FWorldContext& WorldContext,
                                               const FGameFeatureStateChangeContext& ChangeContext)
{
	const auto World = WorldContext.World();
	const auto GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
	const auto ComponentManager = UGameInstance::GetSubsystem<
		UGameFrameworkComponentManager>(GameInstance);

	if (!GameInstance || !World || !World->IsGameWorld() || !ComponentManager) { return; }

	const auto ExtensionRequestHandle = ComponentManager->AddExtensionHandler(
		ABaseHud::StaticClass(),
		UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
			this, &ThisClass::HandleActorExtension, ChangeContext));

	ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
}

void UGameFeatureAction_AddWidgets::HandleActorExtension(AActor* Actor, const FName EventName,
                                                         const FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved ||
		EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveWidgets(Actor, ActiveData);
		return;
	}
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded ||
		EventName == UGameFrameworkComponentManager::NAME_GameActorReady) { AddWidgets(Actor, ActiveData); }
}


void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	ABaseHud* HUD = CastChecked<ABaseHud>(Actor);
	if (!HUD->GetOwningPlayerController()) { return; }
	const auto LocalPlayer = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player);
	if (!LocalPlayer) { return; }

	FPerActorData& ActorData = ActiveData.ActorData.FindOrAdd(HUD);

	for (const FHUDLayoutRequest& Entry : Layout)
	{
		if (const auto ConcreteWidgetClass = Entry.LayoutClass.Get())
		{
			ActorData.LayoutsAdded.Add(
				UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, Entry.LayerID, ConcreteWidgetClass));
		}
	}

	UUIExtensionSubsystem* ExtensionSubsystem = HUD->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
	for (const auto& [WidgetClass, SlotID] : Widgets)
	{
		ActorData.ExtensionHandles.Add(
			ExtensionSubsystem->RegisterExtensionAsWidgetForContext(SlotID,
			                                                        LocalPlayer,
			                                                        WidgetClass.Get(),
			                                                        -1));
	}
}


void UGameFeatureAction_AddWidgets::RemoveWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	ABaseHud* HUD = CastChecked<ABaseHud>(Actor);
	if (!HUD->GetOwningPlayerController()) { return; }

	// Only unregister if this is the same HUD actor that was registered, there can be multiple active at once on the client
	FPerActorData* ActorData = ActiveData.ActorData.Find(HUD);
	if (!ActorData) { return; }

	for (TWeakObjectPtr<UCommonActivatableWidget>& AddedLayout : ActorData->LayoutsAdded)
	{
		if (AddedLayout.IsValid()) { AddedLayout->DeactivateWidget(); }
	}

	for (FUIExtensionHandle& Handle : ActorData->ExtensionHandles) { Handle.Unregister(); }
	ActiveData.ActorData.Remove(HUD);
}

void UGameFeatureAction_AddWidgets::Reset(FPerContextData& ActiveData)
{
	ActiveData.ComponentRequests.Empty();

	for (TPair<FObjectKey, FPerActorData>& Pair : ActiveData.ActorData)
	{
		for (FUIExtensionHandle& Handle : Pair.Value.ExtensionHandles) { Handle.Unregister(); }
	}

	ActiveData.ActorData.Empty();
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddWidgets::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);

	{
		int32 EntryIndex = 0;
		for (const auto& [LayoutClass, LayerID] : Layout)
		{
			if (LayoutClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(
					LOCTEXT("LayoutHasNullClass", "Null WidgetClass at index {0} in Layout"),
					FText::AsNumber(EntryIndex)));
			}

			if (!LayerID.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(
					LOCTEXT("LayoutHasNoTag", "LayerID is not set at index {0} in Widgets"),
					FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	{
		int32 EntryIndex = 0;
		for (const auto& [WidgetClass, SlotID] : Widgets)
		{
			if (WidgetClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(
					LOCTEXT("EntryHasNullClass", "Null WidgetClass at index {0} in Widgets"),
					FText::AsNumber(EntryIndex)));
			}

			if (!SlotID.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(
					LOCTEXT("EntryHasNoTag", "SlotID is not set at index {0} in Widgets"),
					FText::AsNumber(EntryIndex)));
			}
			++EntryIndex;
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
