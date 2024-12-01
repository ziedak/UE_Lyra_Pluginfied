// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InputConfig: ModuleRules
{
	public InputConfig(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;				

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"GameplayTags", 
				"EnhancedInput",
				//"CommonUI",
			});
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				
				//"CommonInput"
			});
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});
	}
}
