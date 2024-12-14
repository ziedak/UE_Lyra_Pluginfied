// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameSettings : ModuleRules
{
	public GameSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"CommonInput",
				"CommonUI",
				"GameplayTags"
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"ApplicationCore",
				"PropertyPath",
				"DeveloperSettings"
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}