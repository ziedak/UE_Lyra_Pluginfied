// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingFilterState.h"
#include "GameSetting.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingFilterState)

#define LOCTEXT_NAMESPACE "GameSetting"

class FSettingFilterExpressionContext : public ITextFilterExpressionContext
{
public:
	explicit FSettingFilterExpressionContext(const UGameSetting& InSetting) : Setting(InSetting)
	{
	}

	virtual bool TestBasicStringExpression(const FTextFilterString& InValue,
	                                       const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		return TextFilterUtils::TestBasicStringExpression(Setting.GetDescriptionPlainText(), InValue,
		                                                  InTextComparisonMode);
	}

	virtual bool TestComplexExpression(const FName& InKey, const FTextFilterString& InValue,
	                                   const ETextFilterComparisonOperation InComparisonOperation,
	                                   const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		return false;
	}

private:
	/** Setting being filtered. */
	const UGameSetting& Setting;
};

//--------------------------------------
// FGameSettingFilterState
//--------------------------------------

FGameSettingFilterState::FGameSettingFilterState()
	: SearchTextEvaluator(ETextFilterExpressionEvaluatorMode::BasicString)
{
}

void FGameSettingFilterState::AddSettingToRootList(UGameSetting* InSetting)
{
	SettingAllowList.Add(InSetting);
	SettingRootList.Add(InSetting);
}

void FGameSettingFilterState::AddSettingToAllowList(UGameSetting* InSetting)
{
	SettingAllowList.Add(InSetting);
}

void FGameSettingFilterState::SetSearchText(const FString& InSearchText)
{
	SearchTextEvaluator.SetFilterText(FText::FromString(InSearchText));
}

bool FGameSettingFilterState::DoesSettingPassFilter(const UGameSetting& InSetting) const
{
	const FGameSettingEditableState& EditableState = InSetting.GetEditState();

	if ((!bIncludeHidden && !EditableState.IsVisible()) ||
		(!bIncludeDisabled && !EditableState.IsEnabled()) ||
		(!bIncludeResetTable && !EditableState.IsResetTable()))
	
		return false;
	
	

	if (SettingAllowList.Num() > 0 && !SettingAllowList.Contains(&InSetting))
	{
		const UGameSetting* NextSetting = &InSetting;
		while (const UGameSetting* Parent = NextSetting->GetSettingParent())
		{
			if (SettingAllowList.Contains(Parent))
				return true;
			
			NextSetting = Parent;
		}
		return false;
	}
	// TODO more filters...
	// Always search text last, it's generally the most expensive filter.
	return SearchTextEvaluator.TestTextFilter(FSettingFilterExpressionContext(InSetting));
	
}

//--------------------------------------
// FGameSettingsEditableState
//--------------------------------------

void FGameSettingEditableState::Hide(const FString& DevReason)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DevReason.IsEmpty(), TEXT("To hide a setting, you must provide a developer reason."));
#endif

	bVisible = false;

#if !UE_BUILD_SHIPPING
	HiddenReasons.Add(DevReason);
#endif
}

void FGameSettingEditableState::Disable(const FText& Reason)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!Reason.IsEmpty(),
	                 TEXT("To disable a setting, you must provide a reason that we can show players."));
#endif

	bEnabled = false;
	DisabledReasons.Add(Reason);
}

void FGameSettingEditableState::DisableOption(const FString& Option)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DisabledOptions.Contains(Option), TEXT("You've already disabled this option."));
#endif

	DisabledOptions.Add(Option);
}


#undef LOCTEXT_NAMESPACE
