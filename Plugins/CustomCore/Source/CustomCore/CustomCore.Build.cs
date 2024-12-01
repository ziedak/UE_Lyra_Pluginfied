// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomCore : ModuleRules
{
	public CustomCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"Core",
				"CoreOnline",// This module includes core functionalities related to online features.
				"CoreUObject",// This module provides the UObject system, which is the base class for most of the objects in Unreal Engine.
				"GameplayAbilities",
				"DTLSHandlerComponent", // This module handles DTLS (Datagram Transport Layer Security) 
				"CommonLoadingScreen",  // This module provides functionalities for common loading screens.
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CommonGame", // This module provides common game functionalities.
				"CommonUser", // This module handles common user-related functionalities.
				"DeveloperSettings", // This module allows for the creation and management of developer settings.
				"EngineSettings", // This module manages engine-specific settings.
				"GameFeatures",// This module supports the implementation of game features.
				"GameplayMessageRuntime", // This module handles runtime gameplay messaging.
				"GameplayTags",// This module allows for the use of gameplay tags, which are useful for categorizing and managing game elements.
				"ModularGameplay",// This module supports modular gameplay features.
				"ModularGameplayActors",// This module extends modular gameplay to actors.
				"NetCore", // This module includes core networking functionalities.
				"Slate", // This module is the user interface framework for Unreal Engine, used for creating in-game and editor UI.
				"SlateCore", // This module provides the core functionalities for the Slate UI framework.
				"UMG", // The Unreal Motion Graphics module is used for creating complex user interfaces. 
			}
		);

		PublicIncludePaths.AddRange(
			new string[] { }
		);
		PrivateIncludePaths.AddRange(
			new string[] { }
		);
		DynamicallyLoadedModuleNames.AddRange(
			new string[] { }
		);
	}
}