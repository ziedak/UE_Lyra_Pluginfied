// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingValueScalarDynamic.h"

#include "DataSource/GameSettingDataSource.h"
#include "UObject/WeakObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingValueScalarDynamic)

#define LOCTEXT_NAMESPACE "GameSetting"

//////////////////////////////////////////////////////////////////////////
// SettingScalarFormats
//////////////////////////////////////////////////////////////////////////

static FText PercentFormat = LOCTEXT("PercentFormat", "{0}%");

UGameSettingValueScalarDynamic* UGameSettingValueScalarDynamic::CreateSettings(const FName& DevName,
                                                                               const FText& DisplayName,
                                                                               const FText& Description,
                                                                               const TSharedRef<FGameSettingDataSource>&
                                                                               Getter,
                                                                               const TSharedRef<FGameSettingDataSource>&
                                                                               Setter,
                                                                               const float DefaultValue,
                                                                               const FSettingScalarFormatFunction&
                                                                               DisplayFormat,
                                                                               const TSharedRef<
	                                                                               FGameSettingEditCondition>&
                                                                               EditCondition)
{
	const auto Setting = NewObject<UGameSettingValueScalarDynamic>();
	Setting->SetDevName(DevName);
	Setting->SetDisplayName(DisplayName);
	Setting->SetDescriptionRichText(Description);
	Setting->SetDynamicGetter(Getter);
	Setting->SetDynamicSetter(Setter);
	Setting->SetDefaultValue(DefaultValue);
	Setting->SetDisplayFormat(DisplayFormat);
	Setting->AddEditCondition(EditCondition);

	return Setting;
}

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::Raw([](const double SourceValue, double NormalizedValue)
{
	return FText::AsNumber(SourceValue);
});

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::RawOneDecimal(
	[](const double SourceValue, double NormalizedValue)
	{
		FNumberFormattingOptions FormattingOptions;
		FormattingOptions.MinimumIntegralDigits = 1;
		FormattingOptions.MinimumFractionalDigits = 1;
		FormattingOptions.MaximumFractionalDigits = 1;
		return FText::AsNumber(SourceValue, &FormattingOptions);
	});

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::RawTwoDecimals(
	[](const double SourceValue, double NormalizedValue)
	{
		FNumberFormattingOptions FormattingOptions;
		FormattingOptions.MinimumIntegralDigits = 1;
		FormattingOptions.MinimumFractionalDigits = 2;
		FormattingOptions.MaximumFractionalDigits = 2;
		return FText::AsNumber(SourceValue, &FormattingOptions);
	});

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::SourceAsInteger(
	[](const double SourceValue, double NormalizedValue)
	{
		FNumberFormattingOptions FormattingOptions;
		FormattingOptions.MinimumIntegralDigits = 1;
		FormattingOptions.MinimumFractionalDigits = 0;
		FormattingOptions.MaximumFractionalDigits = 0;
		return FText::AsNumber(SourceValue, &FormattingOptions);
	});

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::ZeroToOnePercent(
	[](double SourceValue, const double NormalizedValue)
	{
		return FText::Format(PercentFormat, static_cast<int32>(FMath::RoundHalfFromZero(100.0 * NormalizedValue)));
	});

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::ZeroToOnePercent_OneDecimal(
	[](double SourceValue, const double NormalizedValue)
	{
		const FNumberFormattingOptions& FormattingOptions = GetOneDecimalFormattingOptions();
		const double NormalizedValueTo100_0 = FMath::RoundHalfFromZero(1000.0 * NormalizedValue);
		return FText::Format(PercentFormat, FText::AsNumber(NormalizedValueTo100_0 / 10.0, &FormattingOptions));
	});

const FNumberFormattingOptions& UGameSettingValueScalarDynamic::GetOneDecimalFormattingOptions()
{
	static FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MinimumFractionalDigits = 1;
	FormattingOptions.MaximumFractionalDigits = 1;

	return FormattingOptions;
}

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::SourceAsPercent1(
	[](const double SourceValue, double NormalizedValue)
	{
		return FText::Format(PercentFormat, static_cast<int32>(FMath::RoundHalfFromZero(100.0 * SourceValue)));
	});

FSettingScalarFormatFunction UGameSettingValueScalarDynamic::SourceAsPercent100(
	[](const double SourceValue, double NormalizedValue)
	{
		return FText::Format(PercentFormat, static_cast<int32>(FMath::RoundHalfFromZero(SourceValue)));
	});

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueScalarDynamic
//////////////////////////////////////////////////////////////////////////

void UGameSettingValueScalarDynamic::Startup()
{
	// Should I also do something with Setter?
	Getter->Startup(LocalPlayer, FSimpleDelegate::CreateUObject(this, &ThisClass::OnDataSourcesReady));
}

void UGameSettingValueScalarDynamic::OnDataSourcesReady()
{
	StartupComplete();
}

void UGameSettingValueScalarDynamic::OnInitialized()
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(DisplayFormat, TEXT("%s: Has no DisplayFormat set.  Please call SetDisplayFormat."),
	                 *GetDevName().ToString());
#endif

#if !UE_BUILD_SHIPPING
	ensureAlways(Getter);
	ensureAlwaysMsgf(Getter->Resolve(LocalPlayer),
	                 TEXT(
		                 "%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties?"
	                 ), *GetDevName().ToString(), *Getter->ToString());
	ensureAlways(Setter);
	ensureAlwaysMsgf(Setter->Resolve(LocalPlayer),
	                 TEXT(
		                 "%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties?"
	                 ), *GetDevName().ToString(), *Setter->ToString());
#endif

	Super::OnInitialized();
}

void UGameSettingValueScalarDynamic::StoreInitial()
{
	InitialValue = GetValue();
}

void UGameSettingValueScalarDynamic::ResetToDefault()
{
	if (DefaultValue.IsSet())
		SetValue(DefaultValue.GetValue(), EGameSettingChangeReason::ResetToDefault);
}

void UGameSettingValueScalarDynamic::RestoreToInitial()
{
	SetValue(InitialValue, EGameSettingChangeReason::RestoreToInitial);
}


double UGameSettingValueScalarDynamic::GetValue() const
{
	const FString OutValue = Getter->GetValueAsString(LocalPlayer);

	double Value;
	LexFromString(Value, *OutValue);

	return Value;
}

void UGameSettingValueScalarDynamic::SetValue(double InValue, const EGameSettingChangeReason Reason)
{
	InValue = FMath::RoundHalfFromZero(InValue / SourceStep);
	InValue = InValue * SourceStep;

	if (Minimum.IsSet())
		InValue = FMath::Max(Minimum.GetValue(), InValue);

	if (Maximum.IsSet())
		InValue = FMath::Min(Maximum.GetValue(), InValue);

	const FString StringValue = LexToString(InValue);
	Setter->SetValue(LocalPlayer, StringValue);

	NotifySettingChanged(Reason);
}

FText UGameSettingValueScalarDynamic::GetFormattedText() const
{
	const double SourceValue = GetValue();
	const double NormalizedValue = GetValueNormalized();

	return DisplayFormat(SourceValue, NormalizedValue);
}

#undef LOCTEXT_NAMESPACE
