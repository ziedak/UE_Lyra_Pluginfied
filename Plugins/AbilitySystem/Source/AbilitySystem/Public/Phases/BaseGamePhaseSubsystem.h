#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "Enums/PhaseTagMatchType.h"


#include "BaseGamePhaseSubsystem.generated.h"

template <typename T> class TSubclassOf;

class UBaseGamePhaseAbility;
class UBaseAbilitySystemComponent;
class UObject;
struct FFrame;
struct FGameplayAbilitySpecHandle;
struct FPhaseObserver;
struct FBaseGamePhaseEntry;

// DYNAMIC_DELEGATE can be called from  BP
DECLARE_DYNAMIC_DELEGATE_OneParam(FBaseGamePhaseDynamicDelegate, const UBaseGamePhaseAbility*, Phase);
DECLARE_DELEGATE_OneParam(FBaseGamePhaseDelegate, const UBaseGamePhaseAbility* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FBaseGamePhaseTagDynamicDelegate, const FGameplayTag&, PhaseTag);
DECLARE_DELEGATE_OneParam(FBaseGamePhaseTagDelegate, const FGameplayTag& PhaseTag);

/** Subsystem for managing Base's game phases using gameplay tags in a nested manner, which allows parent and child
 * phases to be active at the same time, but not sibling phases.
 * Example:  Game.Playing and Game.Playing.WarmUp can coexist, but Game.Playing and Game.ShowingScore cannot.
 * When a new phase is started, any active phases that are not ancestors will be ended.
 * Example: if Game.Playing and Game.Playing.CaptureTheFlag are active when Game.Playing.PostGame is started,
 *     Game.Playing will remain active, while Game.Playing.CaptureTheFlag will end.
 */
UCLASS()
class UBaseGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UBaseGamePhaseSubsystem();

	virtual void PostInitialize() override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UBaseAbilitySystemComponent* GetBaseAbilitySystemComponent() const;

	//TODO Should we have a way to start a phase with a delay? Or should that be handled by the phase itself ?
	void StartPhase(const TSubclassOf<UBaseGamePhaseAbility>& PhaseAbility, const FBaseGamePhaseDelegate& PhaseEndedCallback = FBaseGamePhaseDelegate());

	//TODO Return a handle so folks can delete these.  They will just grow until the world resets.
	//TODO Should we just occasionally clean these observers up?  It's not as if everyone will properly unhook them even if there is a handle.
	void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FBaseGamePhaseTagDelegate& WhenPhaseActive);
	void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FBaseGamePhaseTagDelegate& WhenPhaseEnd);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	
	void OnBeginPhase(const UBaseGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	void OnEndPhase(const UBaseGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<UBaseGamePhaseAbility> Phase, const FBaseGamePhaseDynamicDelegate& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FBaseGamePhaseTagDynamicDelegate WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FBaseGamePhaseTagDynamicDelegate WhenPhaseEnd);


private:
	struct FBaseGamePhaseEntry
	{
	public:
		FGameplayTag PhaseTag;
		FBaseGamePhaseDelegate PhaseEndedCallback;
	};
	TMap<FGameplayAbilitySpecHandle, FBaseGamePhaseEntry> ActivePhaseMap;

	struct FPhaseObserver
	{
	public:
		FORCEINLINE	bool IsMatch(const FGameplayTag& ComparePhaseTag) const {
			switch (MatchType)
			{
			case EPhaseTagMatchType::ExactMatch:
				return ComparePhaseTag == PhaseTag;
			case EPhaseTagMatchType::PartialMatch:
				return ComparePhaseTag.MatchesTag(PhaseTag);
				/*case EPhaseTagMatchType::ParentMatch:
					return ComparePhaseTag.MatchesTag(PhaseTag) || ComparePhaseTag.IsParentTag(PhaseTag);
				case EPhaseTagMatchType::ChildMatch:
					return ComparePhaseTag.MatchesTag(PhaseTag) || PhaseTag.IsParentTag(ComparePhaseTag);*/
			}
			return false;
		};

		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FBaseGamePhaseTagDelegate PhaseCallback;
	};

	TArray<FPhaseObserver> PhaseStartObservers;
	TArray<FPhaseObserver> PhaseEndObservers;

	friend class UBaseGamePhaseAbility;
};
