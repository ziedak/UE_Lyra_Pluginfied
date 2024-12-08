// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameplayMessageRuntime : ModuleRules
{
	public GameplayMessageRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"Engine",
				"GameplayTags"
			});

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject", "GameplayAbilities"
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}