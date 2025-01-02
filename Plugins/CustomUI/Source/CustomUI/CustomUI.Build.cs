// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomUI : ModuleRules
{
	public CustomUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CommonUI",
				"CommonUser", "GameSettings", "CommonLoadingScreen",
				"ModularGameplay" // This module supports modular gameplay features.
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate", "EnhancedInput", "CommonInput", "InputCore",
				"SlateCore", "CommonGame", "AsyncMixin", "ControlFlows", "UMG", "GameplayTags"
				// ... add private dependencies that you statically link with here ...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}