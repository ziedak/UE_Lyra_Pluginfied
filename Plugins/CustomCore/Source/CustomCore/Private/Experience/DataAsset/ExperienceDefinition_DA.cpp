// Fill out your copyright notice in the Description page of Project Settings.


#include "Experience/DataAsset/ExperienceDefinition_DA.h"
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(ExperienceDefinition_DA)

#define LOCTEXT_NAMESPACE "SystemCore"


#if WITH_EDITOR
EDataValidationResult UExperienceDefinition_DA::IsDataValid(FDataValidationContext& Context) const
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

	// Make sure users didn't subclass from a BP of this (it's fine and expected to subclass once in BP, just not twice)
	if (GetClass()->IsNative())
		return Result;

	const UClass* ParentClass = GetClass()->GetSuperClass();

	// Find the native parent
	const UClass* FirstNativeParent = ParentClass;
	while ((FirstNativeParent != nullptr) && !FirstNativeParent->IsNative())
	{
		FirstNativeParent = FirstNativeParent->GetSuperClass();
	}

	if (FirstNativeParent == ParentClass)
		return Result;

	// Inheritance not allowed
	Context.AddError(FText::Format(LOCTEXT("Experience_Inheritance_Is_Unsupported",
	                                       "Blueprint subclasses of Blueprint experiences is not currently supported (use composition via ActionSets instead). Parent class was {0} but should be {1}."),
	                               FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
	                               FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))
	));
	return EDataValidationResult::Invalid;
}
#endif

#if WITH_EDITORONLY_DATA
void UExperienceDefinition_DA::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : GameFeatureActions)
	{
		if (Action)
			Action->AddAdditionalAssetBundleData(AssetBundleData);
	}
}
#endif // WITH_EDITORONLY_DATA

#undef LOCTEXT_NAMESPACE
