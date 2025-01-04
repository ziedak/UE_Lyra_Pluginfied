// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Gas : ModuleRules
{
	public Gas(ReadOnlyTargetRules target) : base(target)
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
			"CommonGame", "CustomUI", "CommonLoadingScreen"
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
			"UIExtension", // This module allows for extending the user interface.
			"ControlFlows", "GameSettings",
			"Logger",
			"CustomCamera"
		});

		//custom plugin dependencies
		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// Generate compile errors if using DrawDebug functions in test/shipping builds.
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

		// Basic setup for External RPC Framework.
		// Functionality within framework will be stripped in shipping to remove vulnerabilities.
		PrivateDependencyModuleNames.Add("ExternalRpcRegistry");
		PrivateDependencyModuleNames.Add("HTTPServer"); // Dependency for ExternalRpcRegistry
		if (target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("WITH_RPC_REGISTRY=0");
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=0");
		}
		else
		{
			PublicDefinitions.Add("WITH_RPC_REGISTRY=1");
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=1");
		}

		SetupGameplayDebuggerSupport(target);
		// Enable Iris support.
		SetupIrisSupport(target);
	}
}