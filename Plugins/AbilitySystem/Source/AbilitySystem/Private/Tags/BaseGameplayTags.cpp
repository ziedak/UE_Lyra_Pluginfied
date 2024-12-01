// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/BaseGameplayTags.h"

namespace StateTags
{
	UE_DEFINE_GAMEPLAY_TAG(DEAD, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(DEATH_DYING, "State.Death_Dying");
	UE_DEFINE_GAMEPLAY_TAG(DEATH_DEAD, "State.Death_Dead");
	UE_DEFINE_GAMEPLAY_TAG(REMOVE_ON_DEATH, "State.RemoveOnDeath");
	UE_DEFINE_GAMEPLAY_TAG(KNOCKED_DOWN, "State.KnockedDown");
	UE_DEFINE_GAMEPLAY_TAG(INTERACTING, "State.Interacting");
	UE_DEFINE_GAMEPLAY_TAG(INTERACTING_REMOVAL, "State.InteractingRemoval");
}

namespace DataTags
{
	UE_DEFINE_GAMEPLAY_TAG(Damage, "Data.Damage");
}
namespace AbilityTags
{
	UE_DEFINE_GAMEPLAY_TAG(ABILITY, "Ability");
	UE_DEFINE_GAMEPLAY_TAG(SIMPLE_FAILURE_MESSAGE, "Ability.UserFacingSimpleActivateFail.Message");
	UE_DEFINE_GAMEPLAY_TAG(PLAY_MONTAGE_FAILURE_MESSAGE, "Ability.PlayMontageOnActivateFail.Message");
	UE_DEFINE_GAMEPLAY_TAG(NOT_CANCELED_BY_STUN, "Ability.NotCanceledByStun");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(BEHAVIOR_SURVIVES_DEATH, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ACTIVATE_FAIL_IS_DEAD, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ACTIVATE_FAIL_COOLDOWN, "Ability.ActivateFail.OnCooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ACTIVATE_FAIL_COST, "Ability.ActivateFail.CantAffordCost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ACTIVATE_FAIL_TAGS_BLOCKED, "Ability.ActivateFail.BlockedByTags", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ACTIVATE_FAIL_TAGS_MISSING, "Ability.ActivateFail.MissingTags", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ACTIVATE_FAIL_NETWORKING, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ACTIVATE_FAIL_ACTIVATION_GROUP, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");
}
namespace InputTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MOVE, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(LOOK_MOUSE, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(LOOK_STICK, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CROUCH, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AUTORUN, "InputTag.AutoRun", "Auto-run input.");
}

namespace BaseGameplayTags {
	UE_DEFINE_GAMEPLAY_TAG(DAMAGE, "Gameplay.Damage");
	UE_DEFINE_GAMEPLAY_TAG(DAMAGE_IMMUNITY, "Gameplay.DamageImmunity");
	UE_DEFINE_GAMEPLAY_TAG(DAMAGE_SELF_DESTRUCT, "Gameplay.Damage.SelfDestruct");
	UE_DEFINE_GAMEPLAY_TAG(FELL_OUT_OF_WORLD, "Gameplay.Damage.FellOutOfWorld");
	UE_DEFINE_GAMEPLAY_TAG(GAS_DAMAGE_MESSAGE, "Gas.Damage.Message");
	//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Elimination_Message, "Gas.Elimination.Message");
}
namespace GameplayInputTags {
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ABILITY_INPUT_BLOCKED, "Gameplay.AbilityInputBlocked", " Clear Ability Input");
}
namespace GameplayEventTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DEATH, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(RESET, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(REQUEST_RESET, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");
}
namespace SetByCallerTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DAMAGE, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HEAL, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");
}
namespace CheatTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GODMODE, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(UNLIMITED_HEALTH, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");
}
namespace StatusTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(CROUCHING, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AUTORUNNING, "Status.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DEATH, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DEATH_DYING, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(DEATH_DEAD, "Status.Death.Dead", "Target has finished the death process.");
}

namespace DebuffTags
{
	UE_DEFINE_GAMEPLAY_TAG(STUN, "Debuff.Stun")
}
namespace MovementTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MODE_WALKING, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MODE_NAV_WALKING, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MODE_FALLING, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MODE_SWIMMING, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MODE_FLYING, "Movement.Mode.Flying", "Default Character movement tag");

	// When extending Lyra, you can create your own movement modes but you need to update GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MODE_CUSTOM, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See LyraGameplayTags::CustomMovementModeTagMap.");


	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, MODE_WALKING },
		{ MOVE_NavWalking, MODE_NAV_WALKING },
		{ MOVE_Falling, MODE_FALLING },
		{ MOVE_Swimming, MODE_SWIMMING },
		{ MOVE_Flying, MODE_FLYING },
		{ MOVE_Custom, MODE_CUSTOM }
	};

	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		// Fill these in with your custom modes
	};


}

FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
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
				UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
				Tag = TestTag;
				break;
			}
		}
	}

	return Tag;
}