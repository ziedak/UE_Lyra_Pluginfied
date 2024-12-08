// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameplayMessageNodes : ModuleRules
{
	public GameplayMessageNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"KismetCompiler",
				"PropertyEditor",
				"GameplayMessageRuntime",
				"UnrealEd"
			}
		);

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"BlueprintGraph"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
			}
		);

		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);
	}
}