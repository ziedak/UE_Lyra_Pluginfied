// This particular macro is named STARTUP_JOB_WEIGHTED, and it takes two parameters : JobFunc and JobWeight.
// The macro expands to a function call StartupJobs.Add() with three arguments.
//		The first argument is an instance of the FBaseAssetManagerStartupJob class,
//		created using the constructor FBaseAssetManagerStartupJob(#JobFunc, [this](const FBaseAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle) {JobFunc; }, JobWeight).
// Let's break down the constructor arguments:
// �	#JobFunc is a string literal that represents the name of the JobFunc parameter.
//		The # operator is used to convert the parameter into a string.
// �	The second argument is a lambda function that takes two parameters : 
//		const FBaseAssetManagerStartupJob & StartupJob and TSharedPtr<FStreamableHandle>&LoadHandle.
//		This lambda function is defined using a capture list[this] to capture the current object instance.
//		Inside the lambda function, JobFunc is called as a statement.
// �	The third argument is the JobWeight parameter.
// when the STARTUP_JOB_WEIGHTED macro is used,
//	it adds a new instance of FBaseAssetManagerStartupJob to the StartupJobs collection, with the provided JobFunc, 
//	a lambda function that calls JobFunc, 
//	and the specified JobWeight.
#pragma once


#define STARTUP_JOB_WEIGHTED(JobFunc, JobWeight) StartupJobs.Add(FBaseAssetManagerStartupJob(#JobFunc, [this](const FBaseAssetManagerStartupJob& StartupJob, TSharedPtr<FStreamableHandle>& LoadHandle){JobFunc;}, JobWeight))
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc, 1.f)
