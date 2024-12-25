#include "GameFeatures/GameFeatureAction_SplitscreenConfig.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_SplitscreenConfig)

#define LOCTEXT_NAMESPACE "GameFeatures"


TMap<FObjectKey, int32> UGameFeatureAction_SplitscreenConfig::GlobalDisableVotes;

void UGameFeatureAction_SplitscreenConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	for (int32 i = LocalDisableVotes.Num() - 1; i >= 0; i--)
	{
		if (!LocalDisableVotes.IsValidIndex(i)) { continue; }

		FObjectKey ViewportKey = LocalDisableVotes[i];
		const auto Gvp = Cast<UGameViewportClient>(ViewportKey.ResolveObjectPtr());
		if (!Gvp) { continue; }

		const auto WorldContext = GEngine->GetWorldContextFromGameViewport(Gvp);
		// Wrong context so ignore it, dead objects count as part of this context
		if (!WorldContext || !Context.ShouldApplyToWorldContext(*WorldContext)) { continue; }

		int32& VoteCount = GlobalDisableVotes[ViewportKey];
		if (VoteCount <= 1)
		{
			GlobalDisableVotes.Remove(ViewportKey);
			Gvp->SetForceDisableSplitscreen(false);
			continue;
		}

		--VoteCount; // Decrement the vote count
		LocalDisableVotes.RemoveAt(i);
	}
}

void UGameFeatureAction_SplitscreenConfig::AddToWorld(const FWorldContext& WorldContext,
                                                      const FGameFeatureStateChangeContext& ChangeContext)
{
	if (!bDisableSplitscreen) { return; }
	const auto GameInstance = WorldContext.OwningGameInstance;
	if (!GameInstance) { return; }
	const auto VC = GameInstance->GetGameViewportClient();
	if (!VC) { return; }

	const FObjectKey ViewportKey(VC);
	LocalDisableVotes.Add(ViewportKey);

	const int32 VoteCount = GlobalDisableVotes.FindOrAdd(ViewportKey) + 1;
	if (VoteCount == 1) { VC->SetForceDisableSplitscreen(true); }
}

#undef LOCTEXT_NAMESPACE
