#pragma once
namespace CheatConsole
{
	static const auto GName_Fixed = FName(TEXT("Fixed"));

	static auto GEnable_Debug_Camera_Cycling = false;
	static FAutoConsoleVariableRef CVarEnableDebugCameraCycling(
		TEXT("Cheat.EnableDebugCameraCycling"),
		GEnable_Debug_Camera_Cycling,
		TEXT("If true then you can cycle the debug camera while running the game."),
		ECVF_Cheat);

	static auto GSTART_IN_GOD_MODE = false;
	static FAutoConsoleVariableRef CVarStartInGodMode(
		TEXT("Cheat.StartInGodMode"),
		GSTART_IN_GOD_MODE,
		TEXT("If true then the God cheat will be applied on begin play"),
		ECVF_Cheat);
};
