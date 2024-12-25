// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gas : ModuleRules
{
	public Gas(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"GameFeatures",
			"InputCore",
			"ModularGameplay",
			"ModularGameplayActors",
			"NetCore",
			"PhysicsCore",
			"AbilitySystem",
			"CustomCore",
			"GameLocalSettings",
			"CommonGame"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CommonUser",
			"CommonInput",
			"CommonUI",
			"GameplayAbilities",
			"GameplayMessageRuntime",
			"AudioMixer",
			"GameplayTags",
			"GameplayTasks",
			"SignificanceManager",
			"Slate",
			"SlateCore",
			"UMG",
			"UIExtension" // This module allows for extending the user interface.
		});

		//custom plugin dependencies
		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// Generate compile errors if using DrawDebug functions in test/shipping builds.
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

		SetupGameplayDebuggerSupport(Target);

		// Enable Iris support.
		SetupIrisSupport(Target);
	}
}