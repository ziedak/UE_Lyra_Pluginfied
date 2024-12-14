// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingValueScalar.h"

#include "GameSettingValueScalarDynamic.generated.h"

struct FNumberFormattingOptions;

class FGameSettingDataSource;
class UObject;

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueScalarDynamic
//////////////////////////////////////////////////////////////////////////

using FSettingScalarFormatFunction = TFunction<FText(double SourceValue, double NormalizedValue)>;

UCLASS()
class GAMESETTINGS_API UGameSettingValueScalarDynamic : public UGameSettingValueScalar
{
	GENERATED_BODY()

public:
	static UGameSettingValueScalarDynamic* CreateSettings(const FName& DevName,
	                                                      const FText& DisplayName,
	                                                      const FText& Description,
	                                                      const TSharedRef<FGameSettingDataSource>&
	                                                      Getter,
	                                                      const TSharedRef<FGameSettingDataSource>&
	                                                      Setter,
	                                                      const float DefaultValue,
	                                                      const FSettingScalarFormatFunction&
	                                                      DisplayFormat,
	                                                      const TSharedRef<FGameSettingEditCondition>&
	                                                      EditCondition);
	static FSettingScalarFormatFunction Raw;
	static FSettingScalarFormatFunction RawOneDecimal;
	static FSettingScalarFormatFunction RawTwoDecimals;
	static FSettingScalarFormatFunction ZeroToOnePercent;
	static FSettingScalarFormatFunction ZeroToOnePercent_OneDecimal;
	static FSettingScalarFormatFunction SourceAsPercent1;
	static FSettingScalarFormatFunction SourceAsPercent100;
	static FSettingScalarFormatFunction SourceAsInteger;

private:
	static const FNumberFormattingOptions& GetOneDecimalFormattingOptions();

public:
	/** UGameSettingValue */
	virtual void Startup() override;
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	/** UGameSettingValueScalar */
	virtual TOptional<double> GetDefaultValue() const override { return DefaultValue; };
	void SetDefaultValue(const double InValue) { DefaultValue = InValue; }

	virtual double GetValue() const override;
	virtual void SetValue(double InValue, EGameSettingChangeReason Reason = EGameSettingChangeReason::Change) override;

	virtual TRange<double> GetSourceRange() const override { return SourceRange; };

	void SetSourceRangeAndStep(const TRange<double>& InRange, const double InStep)
	{
		SourceRange = InRange;
		SourceStep = InStep;
	};

	virtual double GetSourceStep() const override { return SourceStep; };
	virtual FText GetFormattedText() const override;

	/** UGameSettingValueDiscreteDynamic */
	void SetDynamicGetter(const TSharedRef<FGameSettingDataSource>& InGetter) { Getter = InGetter; }
	void SetDynamicSetter(const TSharedRef<FGameSettingDataSource>& InSetter) { Setter = InSetter; }

	void SetDisplayFormat(const FSettingScalarFormatFunction& InDisplayFormat) { DisplayFormat = InDisplayFormat; }

	/**
	 * The SetSourceRangeAndStep defines the actual range the numbers could move in, but often
	 * the true minimum for the user is greater than the minimum source range, so for example, the range
	 * of some slider might be 0..100, but you want to restrict the slider so that while it shows 
	 * a bar that travels from 0 to 100, the user can't set anything lower than some minimum, e.g. 1.
	 * That is the Minimum Limit.
	 */
	void SetMinimumLimit(const TOptional<double>& InMinimum) { Minimum = InMinimum; };

	/**
	 * The SetSourceRangeAndStep defines the actual range the numbers could move in, but rarely
	 * the true maximum for the user is less than the maximum source range, so for example, the range
	 * of some slider might be 0..100, but you want to restrict the slider so that while it shows
	 * a bar that travels from 0 to 100, the user can't set anything lower than some maximum, e.g. 95.
	 * That is the Maximum Limit.
	 */
	void SetMaximumLimit(const TOptional<double>& InMaximum) { Maximum = InMaximum; };

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;

	void OnDataSourcesReady();

	TSharedPtr<FGameSettingDataSource> Getter;
	TSharedPtr<FGameSettingDataSource> Setter;

	TOptional<double> DefaultValue;
	double InitialValue = 0;

	TRange<double> SourceRange = TRange<double>(0, 1);
	double SourceStep = 0.01;
	TOptional<double> Minimum;
	TOptional<double> Maximum;

	FSettingScalarFormatFunction DisplayFormat;
};
