// Copyright Epic Games,Inc.All Rights Reserved.

using UnrealBuildTool;

public class GameLocalSettings : ModuleRules
{
	public GameLocalSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{"GameSubtitles",
				"Core",
				"Engine",
				"GameSettings",
				"CoreUObject",
				"GameplayTags",
				"RHI", // The Render Hardware Interface module abstracts the graphics hardware, providing a common interface for rendering.
				"ApplicationCore" // This module handles the application lifecycle and platform-specific implementations.
			});

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CommonGame",
				"AudioModulation",
				"CommonLoadingScreen",
				
				"DeveloperSettings",
				"AudioMixer",
				"AudioModulation",
				"EnhancedInput",
				"InputCore",
				"CommonUI",
				"CommonInput",
				"UMG",
				"Slate", // This module is the user interface framework for Unreal Engine, used for creating in-game and editor UI.
				"SlateCore" // This module provides the core functionalities for the Slate UI framework.
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}