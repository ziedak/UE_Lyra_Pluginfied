// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Logger : ModuleRules
{
	public Logger(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Engine",
				"DeveloperSettings", // This module allows for the creation and management of developer settings.
				"EngineSettings" // This module manages engine-specific settings.
			}
		);

		PublicIncludePaths.AddRange(
			new string[] { }
		);
		PrivateIncludePaths.AddRange(
			new string[] { }
		);
		DynamicallyLoadedModuleNames.AddRange(
			new string[] { }
		);
	}
}