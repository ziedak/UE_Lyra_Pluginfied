// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AbilitySystem : ModuleRules
{
	public AbilitySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
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
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"CustomCore",
				"GameplayMessageRuntime"
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"PhysicsCore",
				"NetCore",
				"Logger"
				// ... add private dependencies that you statically link with here ...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

		SetupGameplayDebuggerSupport(Target);

		// Enable Iris support.
		SetupIrisSupport(Target);
	}
}