#include "Phases/BaseGamePhaseSubsystem.h"
#include "Phases/BaseGamePhaseAbility.h"
#include "GameplayTagContainer.h"
#include "Enums/PhaseTagMatchType.h"
#include "Component/BaseAbilitySystemComponent.h"
#include "Engine/World.h"
#include "Log/Loggger.h"
#include "GameFramework/GameStateBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BaseGamePhaseSubsystem)

UBaseGamePhaseSubsystem::UBaseGamePhaseSubsystem()
{
}

void UBaseGamePhaseSubsystem::PostInitialize()
{
	Super::PostInitialize();
}

bool UBaseGamePhaseSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (Super::ShouldCreateSubsystem(Outer))
	{
		//UWorld* World = Cast<UWorld>(Outer);
		//check(World);

		//return World->GetAuthGameMode() != nullptr;
		//return nullptr;
		return true;
	}

	return false;
}

UBaseAbilitySystemComponent* UBaseGamePhaseSubsystem::GetBaseAbilitySystemComponent() const
{
	const UWorld* World = GetWorld();
	return World->GetGameState()->FindComponentByClass<UBaseAbilitySystemComponent>();
}

void UBaseGamePhaseSubsystem::StartPhase(const TSubclassOf<UBaseGamePhaseAbility>& PhaseAbility,
                                         const FBaseGamePhaseDelegate& PhaseEndedCallback)
{
	auto ASC = GetBaseAbilitySystemComponent();
	if (!ensure(ASC))
	{
		return;
	}
	// Give the ability and activate it once to start the phase immediately (if it's not already active) and then end it when it's done.
	// This is a simple way to handle phases that are meant to be short-lived.
	// If you want a phase to be long-lived, you should give the ability to the player's ASC and activate it when you want to start the phase.
	// You can then end the phase by calling UBaseAbilitySystemComponent::ClearAbility() with the ability's handle.
	// This is useful for phases that are meant to be long-lived and can be ended by the player or the game.
	// If you want a phase to be long-lived and only end when the game ends, you can give the ability to the player's ASC and activate
	// it when you want to start the phase.

	FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
	const FGameplayAbilitySpecHandle PhaseSpecHandle = ASC->GiveAbilityAndActivateOnce(PhaseSpec);
	const FGameplayAbilitySpec* PhaseFoundSpecPtr = ASC->FindAbilitySpecFromHandle(PhaseSpecHandle);

	if (!PhaseFoundSpecPtr || PhaseFoundSpecPtr->IsActive())
	{
		ULOG_ERROR(LogTemp, "Failed to start phase %s", *PhaseAbility->GetName());
		return;
	}

	// Store the callback so we can call it when the phase ends
	// This is useful for short-lived phases that are started and ended in one frame

	FBaseGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseSpecHandle);
	Entry.PhaseEndedCallback = PhaseEndedCallback;
}

void UBaseGamePhaseSubsystem::WhenPhaseStartsOrIsActive(const FGameplayTag PhaseTag,
                                                        const EPhaseTagMatchType MatchType,
                                                        const FBaseGamePhaseTagDelegate& WhenPhaseActive)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseActive;

	PhaseStartObservers.Add(Observer);

	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

void UBaseGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag,
                                            EPhaseTagMatchType MatchType,
                                            const FBaseGamePhaseTagDelegate& WhenPhaseEnd)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;

	PhaseEndObservers.Add(Observer);
}

bool UBaseGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& Kvp : ActivePhaseMap)
	{
		const FBaseGamePhaseEntry& PhaseEntry = Kvp.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

void UBaseGamePhaseSubsystem::OnBeginPhase(const UBaseGamePhaseAbility* PhaseAbility,
                                           const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const auto Asc = GetBaseAbilitySystemComponent();
	if (!ensure(Asc))
	{
		return;
	}

	const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();
	ULOG_INFO(LogGAS, "Beginning Phase '%s' (%s)", *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	TArray<FGameplayAbilitySpec*> ActivePhases;
	for (const auto& Kvp : ActivePhaseMap)
	{
		if (FGameplayAbilitySpec* Spec = Asc->FindAbilitySpecFromHandle(Kvp.Key))
		{
			ActivePhases.Add(Spec);
		}
	}

	// End any active phases that don't match the incoming phase tag (i.e. they are sub-phases of the incoming phase)
	// and then add the incoming phase to the active phase map.
	// This allows for multiple phases to be active at the same time, as long as they don't conflict with each other.
	// For example, you could have a "Game.Playing" phase and a "Game.Playing.SuddenDeath" phase active at the same time.

	for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
	{
		const UBaseGamePhaseAbility* ActivePhaseAbility = CastChecked<UBaseGamePhaseAbility>(ActivePhase->Ability);
		const FGameplayTag& ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();

		// So if the active phase currently matches the incoming phase tag, we allow it.
		// i.e. multiple gameplay abilities can all be associated with the same phase tag.
		// For example,
		//		You can be in the, Game.Playing, phase, and then start a sub-phase, like Game.Playing.SuddenDeath
		//		Game.Playing phase will still be active, and if someone were to push another one, like,
		//		Game.Playing.ActualSuddenDeath, it would end Game.Playing.SuddenDeath phase, but Game.Playing would
		//		continue.  Similarly if we activated Game.GameOver, all the Game.Playing* phases would end.
		// If the active phase does not match the incoming phase tag, we end it.
		if (!IncomingPhaseTag.MatchesTag(ActivePhaseTag))
		{
			ULOG_INFO(LogGAS, "Ending Phase '%s' (%s)", *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

			FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
			Asc->CancelAbilitiesByFunc(
				[HandleToEnd](const UBaseGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
				{
					return Handle == HandleToEnd;
				}, true);
		}
	}

	// Add the incoming phase to the active phase map so we can track it and end it later if needed.
	// This is useful for short-lived phases that are started and ended in one frame.

	FBaseGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
	Entry.PhaseTag = IncomingPhaseTag;

	// Notify observers that the phase has started
	// This is useful for triggering events or updating UI when a phase starts.
	// Observers can be added using the WhenPhaseStartsOrIsActive function.
	// The observer will be called when the phase starts or is already active, depending on the match type specified.
	// For example, you could have an observer that triggers an event when the "Game.Playing" phase starts,
	// or when any phase that matches "Game.Playing.*" is active.
	for (const FPhaseObserver& Observer : PhaseStartObservers)
	{
		if (Observer.IsMatch(IncomingPhaseTag))
		{
			Observer.PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
		}
	}
}

void UBaseGamePhaseSubsystem::OnEndPhase(const UBaseGamePhaseAbility* PhaseAbility,
                                         const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	// Remove the phase from the active phase map so we don't track it anymore.
	const FGameplayTag EndedPhaseAbility = PhaseAbility->GetGamePhaseTag();
	ULOG_INFO(LogGAS, "Ending Phase '%s' (%s)", *EndedPhaseAbility.ToString(), *GetNameSafe(PhaseAbility));

	const FBaseGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

	ActivePhaseMap.Remove(PhaseAbilityHandle);

	// Notify observers that the phase has ended
	// This is useful for triggering events or updating UI when a phase ends.
	// Observers can be added using the WhenPhaseEnds function.
	// The observer will be called when the phase ends, depending on the match type specified.
	for (const FPhaseObserver& Observer : PhaseEndObservers)
	{
		if (Observer.IsMatch(EndedPhaseAbility))
		{
			Observer.PhaseCallback.ExecuteIfBound(EndedPhaseAbility);
		}
	}
}

bool UBaseGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

#pragma region K2

void UBaseGamePhaseSubsystem::K2_StartPhase(TSubclassOf<UBaseGamePhaseAbility> PhaseAbility,
                                            const FBaseGamePhaseDynamicDelegate& PhaseEndedDelegate)
{
	//Create a weak delegate to call the blueprint delegate when the phase ends and pass it to StartPhase to start the phase
	//and call the delegate when the phase ends.
	// explain purpose of CreateWeakLambda

	//In the K2_StartPhase function, a weak delegate is used to call a blueprint delegate when the phase ends.Let's break down how it is used:
	//	1.	The function takes two parameters : PhaseAbility, which is a subclass of UBaseGamePhaseAbility, and PhaseEndedDelegate, which is a blueprint delegate that will be called when the phase ends.
	//	2.	Inside the function, a FBaseGamePhaseDelegate named PhaseEndedCallback is created.This delegate is responsible for calling the PhaseEndedDelegate when the phase ends.
	//	3.	The CreateWeakLambda function is used to create a weak delegate.This function takes two parameters : the object that the delegate belongs to and a lambda function that will be executed when the delegate is called.
	//	4.	The first parameter of CreateWeakLambda is const_cast<UObject*>(PhaseEndedDelegate.GetUObject()).This retrieves the UObject associated with the PhaseEndedDelegate and casts away the constness, allowing it to be used in the lambda function.
	//	5.	The second parameter of CreateWeakLambda is a lambda function that takes a const UBaseGamePhaseAbility * parameter.Inside the lambda function, the PhaseEndedDelegate is executed using PhaseEndedDelegate.ExecuteIfBound(PhaseAbility).This calls the blueprint delegate and passes the PhaseAbility as a parameter.
	//	6.	The resulting weak delegate, PhaseEndedCallback, is then passed as a parameter to the StartPhase function along with the PhaseAbility.This starts the phase and associates the weak delegate with it.
	//	7.	When the phase ends, the PhaseEndedCallback weak delegate will be called.If the object associated with the delegate still exists, the lambda function will be executed, which in turn calls the PhaseEndedDelegate blueprint delegate.
	//	By using a weak delegate, the K2_StartPhase function ensures that the blueprint delegate is only called if the object associated with it still exists.This helps prevent potential crashes or errors if the object has been destroyed before the phase ends.

	const FBaseGamePhaseDelegate PhaseEndedCallback = FBaseGamePhaseDelegate::CreateWeakLambda(
		const_cast<UObject*>(
			PhaseEndedDelegate.GetUObject()),
		[PhaseEndedDelegate](const UBaseGamePhaseAbility* PhaseAbility)
		{
			PhaseEndedDelegate.ExecuteIfBound(PhaseAbility);
		});

	StartPhase(PhaseAbility, PhaseEndedCallback);
}

void UBaseGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(const FGameplayTag PhaseTag,
                                                           const EPhaseTagMatchType MatchType,
                                                           FBaseGamePhaseTagDynamicDelegate WhenPhaseActive)
{
	const FBaseGamePhaseTagDelegate ActiveDelegate = FBaseGamePhaseTagDelegate::CreateWeakLambda(
		WhenPhaseActive.GetUObject(),
		[WhenPhaseActive](const FGameplayTag& PhaseTag)
		{
			WhenPhaseActive.ExecuteIfBound(PhaseTag);
		});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void UBaseGamePhaseSubsystem::K2_WhenPhaseEnds(const FGameplayTag PhaseTag,
                                               const EPhaseTagMatchType MatchType,
                                               FBaseGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
	const FBaseGamePhaseTagDelegate EndedDelegate = FBaseGamePhaseTagDelegate::CreateWeakLambda(
		WhenPhaseEnd.GetUObject(),
		[WhenPhaseEnd](const FGameplayTag& PhaseTag)
		{
			WhenPhaseEnd.ExecuteIfBound(PhaseTag);
		});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}
#pragma endregion
