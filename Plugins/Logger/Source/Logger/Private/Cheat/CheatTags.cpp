// Fill out your copyright notice in the Description page of Project Settings.


#include "Cheat/CheatTags.h"
#include "GameplayTagsManager.h"

namespace SetByCallerTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DAMAGE, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HEAL, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");
}

namespace CheatTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GOD_MODE, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UNLIMITED_HEALTH, "Cheat.UnlimitedHealth",
	                               "UnlimitedHealth cheat is active on the owner.");
}


FGameplayTag FindTagByString(const FString& TagString, const bool bMatchPartialString)
{
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

	if (!Tag.IsValid() && bMatchPartialString)
	{
		FGameplayTagContainer AllTags;
		Manager.RequestAllGameplayTags(AllTags, true);

		for (const FGameplayTag& TestTag : AllTags)
		{
			if (TestTag.ToString().Contains(TagString))
			{
				UE_LOG(LogTemp, Display,
				       TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString,
				       *TestTag.ToString());
				Tag = TestTag;
				break;
			}
		}
	}

	return Tag;
}
