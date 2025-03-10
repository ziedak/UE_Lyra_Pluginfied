// Fill out your copyright notice in the Description page of Project Settings.


#include "Experience/DataAsset/ExperienceActionSet_DA.h"
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceActionSet_DA)

#define LOCTEXT_NAMESPACE "SystemCore"

#if WITH_EDITOR
EDataValidationResult UExperienceActionSet_DA::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);

	for (auto Idx = 0; Idx < GameFeatureActions.Num(); Idx++)
	{
		const UGameFeatureAction* Action = GameFeatureActions[Idx];

		if (!Action)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(
				LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in GameFeatureActions"), FText::AsNumber(Idx)));

			continue;
		}
		const EDataValidationResult ChildResult = Action->IsDataValid(Context);
		Result = CombineDataValidationResults(Result, ChildResult);
	}

	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void UExperienceActionSet_DA::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : GameFeatureActions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE
