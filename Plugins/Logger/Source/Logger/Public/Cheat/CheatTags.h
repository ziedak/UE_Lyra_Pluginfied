// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "NativeGameplayTags.h"

namespace SetByCallerTags
{
	LOGGER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(DAMAGE);
	LOGGER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HEAL);
}

namespace CheatTags
{
	LOGGER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GOD_MODE);
	LOGGER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UNLIMITED_HEALTH);
}

LOGGER_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);
