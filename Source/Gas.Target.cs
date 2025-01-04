// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.IO;
using EpicGames.Core;
using Microsoft.Extensions.Logging;
using UnrealBuildBase;
using UnrealBuildTool;

public class GasTarget : TargetRules
{
	private static bool _bHasWarnedAboutShared;

	private static readonly Dictionary<string, JsonObject> AllPluginRootJsonObjectsByName = new();

	public GasTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange(new[] { "Gas" });

		ApplySharedLyraTargetSettings(this);
	}

	internal static void ApplySharedLyraTargetSettings(TargetRules target)
	{
		var logger = target.Logger;

		target.DefaultBuildSettings = BuildSettingsVersion.V5;
		target.IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		var bIsTest = target.Configuration == UnrealTargetConfiguration.Test;
		var bIsShipping = target.Configuration == UnrealTargetConfiguration.Shipping;
		var bIsDedicatedServer = target.Type == TargetType.Server;

		if (target.BuildEnvironment == TargetBuildEnvironment.Unique)
		{
			target.ShadowVariableWarningLevel = WarningLevel.Error;

			target.bUseLoggingInShipping = true;

			if (bIsShipping && !bIsDedicatedServer)
				// Make sure that we validate certificates for HTTPS traffic
				target.bDisableUnverifiedCertificates = true;
			// Uncomment these lines to lock down the command line processing
			// This will only allow the specified command line arguments to be parsed
			//Target.GlobalDefinitions.Add("UE_COMMAND_LINE_USES_ALLOW_LIST=1");
			//Target.GlobalDefinitions.Add("UE_OVERRIDE_COMMAND_LINE_ALLOW_LIST=\"-space -separated -list -of -commands\"");
			// Uncomment this line to filter out sensitive command line arguments that you
			// don't want to go into the log file (e.g., if you were uploading logs)
			//Target.GlobalDefinitions.Add("FILTER_COMMANDLINE_LOGGING=\"-some_connection_id -some_other_arg\"");
			if (bIsShipping || bIsTest)
			{
				// Disable reading generated/non-ufs ini files
				target.bAllowGeneratedIniWhenCooked = false;
				target.bAllowNonUFSIniWhenCooked = false;
			}

			if (target.Type != TargetType.Editor)
			{
				// We don't use the path tracer at runtime, only for beauty shots, and this DLL is quite large
				target.DisablePlugins.Add("OpenImageDenoise");
				// Reduce memory use in AssetRegistry always-loaded data, but add more cputime expensive queries
				target.GlobalDefinitions.Add("UE_ASSETREGISTRY_INDIRECT_ASSETDATA_POINTERS=1");
			}

			ConfigureGameFeaturePlugins(target);
			return;
		}

		// !!!!!!!!!!!! WARNING !!!!!!!!!!!!!
		// Any changes in here must not affect PCH generation, or the target
		// needs to be set to TargetBuildEnvironment.Unique

		// This only works in editor or Unique build environments
		if (target.Type == TargetType.Editor)
		{
			ConfigureGameFeaturePlugins(target);
			return;
		}

		// Shared monolithic builds cannot enable/disable plugins or change any options because it tries to re-use the installed engine binaries
		if (_bHasWarnedAboutShared) return;
		_bHasWarnedAboutShared = true;
		logger.LogWarning("GasEOS and dynamic target options are disabled when packaging from an installed version of the engine");
	}

	public static bool ShouldEnableAllGameFeaturePlugins(TargetRules target)
	{
		if (target.Type == TargetType.Editor)
		{
			// With return true, editor builds will build all game feature plugins, but it may or may not load them all.
			// This is so you can enable plugins in the editor without needing to compile code.
			// return true;
		}

		var bIsBuildMachine = Environment.GetEnvironmentVariable("IsBuildMachine") == "1";
		if (bIsBuildMachine)
		{
			// This could be used to enable all plugins for build machines
			// return true;
		}

		// By default, use the default plugin rules as set by the plugin browser in the editor
		// This is important because this code may not be run at all for launcher-installed versions of the engine
		return false;
	}

	// Configures which game feature plugins we want to have enabled
	// This is a fairly simple implementation, but you might do things like build different
	// plugins based on the target release version of the current branch, e.g., enabling 
	// work-in-progress features in main but disabling them in the current release branch.
	public static void ConfigureGameFeaturePlugins(TargetRules target)
	{
		var logger = target.Logger;
		Log.TraceInformationOnce("Compiling GameFeaturePlugins in branch {0}", target.Version.BranchName);

		var bBuildAllGameFeaturePlugins = ShouldEnableAllGameFeaturePlugins(target);

		// Load all the game feature .uplugin descriptors
		var combinedPluginList = new List<FileReference>();

		if (target.ProjectFile != null)
		{
			var gameFeaturePluginRoots = Unreal.GetExtensionDirs(target.ProjectFile.Directory, Path.Combine("Plugins", "GameFeatures"));
			foreach (var searchDir in gameFeaturePluginRoots) combinedPluginList.AddRange(PluginsBase.EnumeratePlugins(searchDir));
		}

		if (combinedPluginList.Count <= 0) return;
		var allPluginReferencesByName = new Dictionary<string, List<string>>();

		foreach (var pluginFile in combinedPluginList)
		{
			if (pluginFile == null || !FileReference.Exists(pluginFile)) continue;

			var bEnabled = false;
			var bForceDisabled = false;
			try
			{
				if (!AllPluginRootJsonObjectsByName.TryGetValue(pluginFile.GetFileNameWithoutExtension(), out var rawObject))
				{
					rawObject = JsonObject.Read(pluginFile);
					AllPluginRootJsonObjectsByName.Add(pluginFile.GetFileNameWithoutExtension(), rawObject);
				}

				// Validate that all GameFeaturePlugins are disabled by default
				// If EnabledByDefault is true and a plugin is disabled the name will be embedded in the executable
				// If this is a problem, enable this warning and change the game feature editor plugin templates to disable EnabledByDefault for new plugins
				if (!rawObject.TryGetBoolField("EnabledByDefault", out var bEnabledByDefault) || bEnabledByDefault)
					logger.LogWarning("GameFeaturePlugin {0}, does not set EnabledByDefault to false. This is required for built-in GameFeaturePlugins.", pluginFile.GetFileNameWithoutExtension());

				// Validate that all GameFeaturePlugins are set to explicitly loaded
				// This is important because game feature plugins expect to be loaded after project startup
				if (!rawObject.TryGetBoolField("ExplicitlyLoaded", out var bExplicitlyLoaded) || bExplicitlyLoaded == false)
					logger.LogWarning("GameFeaturePlugin {0}, does not set ExplicitlyLoaded to true. This is required for GameFeaturePlugins.", pluginFile.GetFileNameWithoutExtension());

				// You could read an additional field here that is project specific, e.g.,
				//string PluginReleaseVersion;
				//if (RawObject.TryGetStringField("MyProjectReleaseVersion", out PluginReleaseVersion))
				//{
				//		bEnabled = SomeFunctionOf(PluginReleaseVersion, CurrentReleaseVersion) || bBuildAllGameFeaturePlugins;
				//}

				if (bBuildAllGameFeaturePlugins)
					// We are in a mode where we want all game feature plugins, except ones we can't load or compile
					bEnabled = true;

				// Prevent using editor-only feature plugins in non-editor builds
				if (rawObject.TryGetBoolField("EditorOnly", out var bEditorOnly))
					if (bEditorOnly && target.Type != TargetType.Editor && !bBuildAllGameFeaturePlugins)
						// The plugin is editor only , we are building a non-editor target, so it is disabled
						bForceDisabled = true;


				// some plugins should only be available in certain branches
				if (rawObject.TryGetStringField("RestrictToBranch", out var restrictToBranch))
				{
					if (!target.Version.BranchName.Equals(restrictToBranch, StringComparison.OrdinalIgnoreCase))
					{
						// The plugin is for a specific branch, and this isn't it
						bForceDisabled = true;
						logger.LogDebug("GameFeaturePlugin {Name} was marked as restricted to other branches. Disabling.", pluginFile.GetFileNameWithoutExtension());
					}
					else
					{
						logger.LogDebug("GameFeaturePlugin {Name} was marked as restricted to this branch. Leaving enabled.", pluginFile.GetFileNameWithoutExtension());
					}
				}

				// Plugins can be marked as NeverBuild which overrides the above
				if (rawObject.TryGetBoolField("NeverBuild", out var bNeverBuild) && bNeverBuild)
				{
					// This plugin was marked to never compile, so don't
					bForceDisabled = true;
					logger.LogDebug("GameFeaturePlugin {Name} was marked as NeverBuild, disabling.", pluginFile.GetFileNameWithoutExtension());
				}

				// Keep track of plugin references for validation later
				if (rawObject.TryGetObjectArrayField("Plugins", out var pluginReferencesArray))
					foreach (var referenceObject in pluginReferencesArray)
					{
						if (!referenceObject.TryGetBoolField("Enabled", out var bRefEnabled) || bRefEnabled != true) continue;

						if (!referenceObject.TryGetStringField("Name", out var pluginReferenceName)) continue;
						var referencerName = pluginFile.GetFileNameWithoutExtension();
						if (!allPluginReferencesByName.ContainsKey(referencerName)) allPluginReferencesByName[referencerName] = new List<string>();

						allPluginReferencesByName[referencerName].Add(pluginReferenceName);
					}
			}
			catch (Exception parseException)
			{
				logger.LogWarning("Failed to parse GameFeaturePlugin file {Name}, disabling. Exception: {1}", pluginFile.GetFileNameWithoutExtension(), parseException.Message);
				bForceDisabled = true;
			}

			// Disabled has priority over enabled
			if (bForceDisabled) bEnabled = false;

			// Print out the final decision for this plugin
			logger.LogDebug("ConfigureGameFeaturePlugins() has decided to {Action} feature {Name}", bEnabled ? "enable" : bForceDisabled ? "disable" : "ignore",
				pluginFile.GetFileNameWithoutExtension());

			// Enable or disable it
			if (bEnabled) target.EnablePlugins.Add(pluginFile.GetFileNameWithoutExtension());

			if (bForceDisabled) target.DisablePlugins.Add(pluginFile.GetFileNameWithoutExtension());
		}

		// If you use something like a release version, consider doing a reference validation to make sure
		// that plugins with sooner release versions don't depend on content with later release versions
	}
}