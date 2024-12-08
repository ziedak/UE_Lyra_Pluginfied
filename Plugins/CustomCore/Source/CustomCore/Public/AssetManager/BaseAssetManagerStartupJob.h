#pragma once

#include "Engine/StreamableManager.h"

DECLARE_DELEGATE_OneParam(FBaseAssetManagerStartupJobSubstepProgress, float /*NewProgress*/);

/** Handles reporting progress from streamable handles */
struct FBaseAssetManagerStartupJob
{
	FBaseAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;
	TFunction<void(const FBaseAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)> JobFunc;
	FString JobName;
	float JobWeight;
	mutable double LastUpdate = 0;

	/** Simple job that is all synchronous */
	FBaseAssetManagerStartupJob(const FString& InJobName,
	                            const TFunction<void(const FBaseAssetManagerStartupJob&,
	                                                 TSharedPtr<FStreamableHandle>&)>& InJobFunc,
	                            const float InJobWeight)
		: JobFunc(InJobFunc)
		  , JobName(InJobName)
		  , JobWeight(InJobWeight)
	{
	}

	/** Perform actual loading, will return a handle if it created one */
	TSharedPtr<FStreamableHandle> DoJob();

	void UpdateSubstepProgress(const float NewProgress) const
	{
		SubstepProgressDelegate.ExecuteIfBound(NewProgress);
	}

	void UpdateSubstepProgressFromStreamable(TSharedRef<FStreamableHandle> StreamableHandle) const
	{
		if (SubstepProgressDelegate.IsBound())
		{
			// StreamableHandle::GetProgress traverses() a large graph and is quite expensive
			const double Now = FPlatformTime::Seconds();
			if (LastUpdate - Now > 1.0 / 60)
			{
				SubstepProgressDelegate.Execute(StreamableHandle->GetProgress());
				LastUpdate = Now;
			}
		}
	}
};
