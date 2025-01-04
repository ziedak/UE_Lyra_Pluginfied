// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomUI : ModuleRules
{
	public CustomUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"ApplicationCore",
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