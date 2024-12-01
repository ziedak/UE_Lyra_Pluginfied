#include "AssetManager/BaseAssetManagerStartupJob.h"

#include"Log/Loggger.h"
//	1.	The function starts by declaring a variable JobStartTime and assigning it the current time in seconds using FPlatformTime::Seconds().
//			This variable will be used to measure the time taken to complete the job.
//	2.	Next, a shared pointer Handle is declared.This pointer will be used to hold an instance of FStreamableHandle.
//	3.	The code then logs a message using the LOG_PRINTF macro, indicating that the startup job is starting.
//			The message includes the name of the job, which is stored in the JobName variable.
//	4.	The line JobFunc(*this, Handle); invokes a callback function called JobFunc and passes* this
//	5.	The code then checks if the Handle is valid by calling the IsValid() function on it.
//	6.	the BindUpdateDelegate() function is called on the Handle object.This function binds an update delegate,
//		which is a callback function that will be called when the streamable handle is updated.
//	7.	The BindUpdateDelegate() function is passed a raw function pointer created using the CreateRaw() function.
//		This raw function pointer points to the member function UpdateSubstepProgressFromStreamable of the current instance of
//		FBaseAssetManagerStartupJob.
//	8.	After binding the update delegate, the code calls WaitUntilComplete() on the Handle object.
//		This function waits until the streamable handle has completed loading with a timeout of 0.0 seconds and does not block the calling thread.
//	9.	Once the loading is complete, the code unbinds the update delegate by calling BindUpdateDelegate() on the Handle object with an
//		empty delegate.
//	10.	Finally, the code logs another message indicating the completion of the startup job.
//		The message includes the name of the job and the time taken to complete the job, which is calculated by subtracting JobStartTime from the current time using FPlatformTime::Seconds().
//	11.	The function returns the Handle object.
//
//	Overall, this code performs a startup job by invoking a function(JobFunc) and handling the loading and completion of a streamable handle(Handle).
//		It also logs messages to provide information about the job's progress and completion.

TSharedPtr<FStreamableHandle> FBaseAssetManagerStartupJob::DoJob()
{
	const double JobStartTime = FPlatformTime::Seconds();
	TSharedPtr<FStreamableHandle> Handle;

	UE_LOG(LogGAS, Display, TEXT("Startup job \"%s\" starting"), *JobName);

	if (!JobFunc)
	{
		UE_LOG(LogGAS, Display, TEXT( "JobFunc is null for job: %s"), *JobName);
		return Handle;
	}

	JobFunc(*this, Handle);

	if (Handle.IsValid())
	{
		Handle->BindUpdateDelegate(
			FStreamableUpdateDelegate::CreateRaw(
				this, &FBaseAssetManagerStartupJob::UpdateSubstepProgressFromStreamable));
		Handle->WaitUntilComplete(0.0f, false);
		Handle->BindUpdateDelegate(FStreamableUpdateDelegate());
	}

	UE_LOG(LogGAS, Display, TEXT( "Startup job:  \"%s\" took %.2f seconds to complete"), *JobName,
	       FPlatformTime::Seconds() - JobStartTime);

	return Handle;
}
