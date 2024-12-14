// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingValueDiscrete.h"

#include "GameSettingValueDiscreteDynamic.generated.h"

class FGameSettingDataSource;
enum class EGameSettingChangeReason : uint8;

struct FContentControlsRules;

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic
//////////////////////////////////////////////////////////////////////////

UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic : public UGameSettingValueDiscrete
{
	GENERATED_BODY()

public:
	/** UGameSettingValue */
	virtual void Startup() override;
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;

	/** UGameSettingValueDiscrete */
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual int32 GetDiscreteOptionDefaultIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;

	/** UGameSettingValueDiscreteDynamic */
	void SetDynamicGetter(const TSharedRef<FGameSettingDataSource>& InGetter) { Getter = InGetter; };
	void SetDynamicSetter(const TSharedRef<FGameSettingDataSource>& InSetter) { Setter = InSetter; };
	void SetDefaultValueFromString(const FString& InOptionValue) { DefaultValue = InOptionValue; };
	void AddDynamicOption(const FString& InOptionValue, const FText& InOptionText);
	void RemoveDynamicOption(const FString& InOptionValue);
	const TArray<FString>& GetDynamicOptions() { return OptionValues; };

	bool HasDynamicOption(const FString& InOptionValue) const { return OptionValues.Contains(InOptionValue); };

	FString GetValueAsString() const;
	void SetValueFromString(const FString& InStringValue);

protected:
	void SetValueFromString(const FString& InStringValue, EGameSettingChangeReason Reason);

	/** UGameSettingValue */
	virtual void OnInitialized() override;

	void OnDataSourcesReady();

	bool AreOptionsEqual(const FString& InOptionA, const FString& InOptionB) const { return InOptionA == InOptionB; };

	TSharedPtr<FGameSettingDataSource> Getter;
	TSharedPtr<FGameSettingDataSource> Setter;

	TOptional<FString> DefaultValue;
	FString InitialValue;

	TArray<FString> OptionValues;
	TArray<FText> OptionDisplayTexts;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Bool
//////////////////////////////////////////////////////////////////////////

UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Bool : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	static
	UGameSettingValueDiscreteDynamic_Bool* CreateSettings(const FName& DevName,
	                                                      const FText& DisplayName,
	                                                      const FText& Description,
	                                                      const TSharedRef<FGameSettingDataSource>&
	                                                      Getter,
	                                                      const TSharedRef<FGameSettingDataSource>&
	                                                      Setter,
	                                                      const bool DefaultValue);

	UGameSettingValueDiscreteDynamic_Bool();

	void SetDefaultValue(bool Value);

	void SetTrueText(const FText& InText);
	void SetFalseText(const FText& InText);

#if !UE_BUILD_SHIPPING
	void SetTrueText(const FString& Value) { SetTrueText(FText::FromString(Value)); }
	void SetFalseText(const FString& Value) { SetFalseText(FText::FromString(Value)); }
#endif
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Number
//////////////////////////////////////////////////////////////////////////

UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Number : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	template <typename NumberType>
	static UGameSettingValueDiscreteDynamic_Number* Create(const FName& DevName,
	                                                       const FText& DisplayName,
	                                                       const FText& Description,
	                                                       const TSharedRef<FGameSettingDataSource>& Getter,
	                                                       const TSharedRef<FGameSettingDataSource>& Setter,
	                                                       const NumberType DefaultValue,
	                                                       const int32 NbOptions)
	{
		UGameSettingValueDiscreteDynamic_Number* Setting = NewObject<UGameSettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(DevName);
		Setting->SetDisplayName(DisplayName);
		Setting->SetDescriptionRichText(Description);
		Setting->SetDynamicGetter(Getter);
		Setting->SetDynamicSetter(Setter);
		Setting->SetDefaultValue(DefaultValue);
		for (int32 Index = 0; Index <= NbOptions; Index++)
		{
			Setting->AddOption(Index, FText::AsNumber(Index));
		}

		return Setting;
	}

	template <typename NumberType>
	void SetDefaultValue(NumberType InValue)
	{
		SetDefaultValueFromString(LexToString(InValue));
	}

	template <typename NumberType>
	void AddOption(NumberType InValue, const FText& InOptionText)
	{
		AddDynamicOption(LexToString(InValue), InOptionText);
	}

	template <typename NumberType>
	NumberType GetValue() const
	{
		const FString ValueString = GetValueAsString();

		NumberType OutValue;
		LexFromString(OutValue, *ValueString);

		return OutValue;
	}

	template <typename NumberType>
	void SetValue(NumberType InValue)
	{
		SetValueFromString(LexToString(InValue));
	}

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Enum
//////////////////////////////////////////////////////////////////////////

UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Enum : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	//@TODO: Add support to convert enums to TMap<EnumType, FText> Collection)
	// {
	// {Enum::Val, LOCTEXT("Enum_Val", "Val")},
	// ...
	// }


//@TODO verify this is the right way to do this
	template <typename EnumType>
	static TMap<EnumType, FText> EnumToMap(const FString& NameSpace)
	{
		TMap<EnumType, FText> Collection;
		const TCHAR* LNameSpace = *NameSpace;
		for (EnumType Elm : TEnumRange<EnumType>())
		{
			const auto LEnumName = StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(Elm));
			const auto LInKey = *FString::Printf(TEXT("%s_%s"), *LEnumName, *LEnumName);
			auto Txt = NSLOCTEXT(NameSpace, InKey, EnumName);
			Collection.Add(Elm, Txt);
		}
		return Collection;
	}

	template <typename EnumType>
	static UGameSettingValueDiscreteDynamic_Enum* CreateEnumSettings(const FName& DevName,
	                                                                 const FText& DisplayName,
	                                                                 const FText& Description,
	                                                                 const TSharedRef<FGameSettingDataSource>& Getter,
	                                                                 const TSharedRef<FGameSettingDataSource>& Setter,
	                                                                 const EnumType DefaultValue,
	                                                                 TMap<EnumType, FText> Collection)
	{
		UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
		Setting->SetDevName(DevName);
		Setting->SetDisplayName(DisplayName);
		Setting->SetDescriptionRichText(Description);
		Setting->SetDynamicGetter(Getter);
		Setting->SetDynamicSetter(Setter);
		Setting->SetDefaultValue(DefaultValue);
		for (auto Element : Collection)
		{
			Setting->AddEnumOption(Element.Key, Element.Value);
		}
		return Setting;
	}


	template <typename EnumType>
	void SetDefaultValue(EnumType InEnumValue)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InEnumValue));
		SetDefaultValueFromString(StringValue);
	}

	template <typename EnumType>
	void AddEnumOption(EnumType InEnumValue, const FText& InOptionText)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InEnumValue));
		AddDynamicOption(StringValue, InOptionText);
	}

	template <typename EnumType>
	EnumType GetValue() const
	{
		const FString Value = GetValueAsString();
		return static_cast<EnumType>(StaticEnum<EnumType>()->GetValueByNameString(Value));
	}

	template <typename EnumType>
	void SetValue(EnumType InEnumValue)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InEnumValue));
		SetValueFromString(StringValue);
	}

protected:
	/** UGameSettingValue */
	virtual void OnInitialized() override;
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Color
//////////////////////////////////////////////////////////////////////////

UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Color : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	void SetDefaultValue(FLinearColor InColor)
	{
		SetDefaultValueFromString(InColor.ToString());
	}

	void AddColorOption(FLinearColor InColor)
	{
		const FColor SRGBColor = InColor.ToFColor(true);
		AddDynamicOption(InColor.ToString(),
		                 FText::FromString(
			                 FString::Printf(TEXT("#%02X%02X%02X"), SRGBColor.R, SRGBColor.G, SRGBColor.B)));
	}

	FLinearColor GetValue() const
	{
		const FString Value = GetValueAsString();

		FLinearColor ColorValue;
		bool bSuccess = ColorValue.InitFromString(Value);
		ensure(bSuccess);

		return ColorValue;
	}

	void SetValue(FLinearColor InColor)
	{
		SetValueFromString(InColor.ToString());
	}
};

//////////////////////////////////////////////////////////////////////////
// UGameSettingValueDiscreteDynamic_Vector2D
//////////////////////////////////////////////////////////////////////////

UCLASS()
class GAMESETTINGS_API UGameSettingValueDiscreteDynamic_Vector2D : public UGameSettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	void SetDefaultValue(const FVector2D& InValue)
	{
		SetDefaultValueFromString(InValue.ToString());
	}

	FVector2D GetValue() const
	{
		FVector2D ValueVector;
		ValueVector.InitFromString(GetValueAsString());
		return ValueVector;
	}

	void SetValue(const FVector2D& InValue)
	{
		SetValueFromString(InValue.ToString());
	}
};
