// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameSettingValueScalar.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GameSettingValueScalar)

#define LOCTEXT_NAMESPACE "GameSetting"

//--------------------------------------
// UGameSettingValueScalar
//--------------------------------------


void UGameSettingValueScalar::SetValueNormalized(const double NormalizedValue)
{
	SetValue(FMath::GetMappedRangeValueClamped(TRange<double>(0, 1), GetSourceRange(), NormalizedValue));
}

double UGameSettingValueScalar::GetValueNormalized() const
{
	return FMath::GetMappedRangeValueClamped(GetSourceRange(), TRange<double>(0, 1), GetValue());
}

#undef LOCTEXT_NAMESPACE
