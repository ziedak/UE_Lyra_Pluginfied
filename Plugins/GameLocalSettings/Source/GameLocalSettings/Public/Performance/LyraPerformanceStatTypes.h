// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/EnumRange.h"

#include "LyraPerformanceStatTypes.generated.h"

//////////////////////////////////////////////////////////////////////

// Way to display the stat
UENUM(BlueprintType)
enum class EStatDisplayMode : uint8
{
	// Don't show this stat
	Hidden,

	// Show this stat in text form
	TextOnly,

	// Show this stat in graph form
	GraphOnly,

	// Show this stat as both text and graph
	TextAndGraph
};

//////////////////////////////////////////////////////////////////////

// Different kinds of stats that can be displayed on-screen
UENUM(BlueprintType)
enum class EDisplayablePerformanceStat : uint8
{
	// stat fps (in Hz)
	ClientFPS,

	// server tick rate (in Hz)
	ServerFPS,

	// idle time spent waiting for vsync or frame rate limit (in seconds)
	IdleTime,

	// Stat unit overall (in seconds)
	FrameTime,

	// Stat unit (game thread, in seconds)
	FrameTime_GameThread,

	// Stat unit (render thread, in seconds)
	FrameTime_RenderThread,

	// Stat unit (RHI thread, in seconds)
	FrameTime_RHIThread,

	// Stat unit (inferred GPU time, in seconds)
	FrameTime_GPU,

	// Network ping (in ms)
	Ping,

	// The incoming packet loss percentage (%)
	PacketLoss_Incoming,

	// The outgoing packet loss percentage (%)
	PacketLoss_Outgoing,

	// The number of packets received in the last second
	PacketRate_Incoming,

	// The number of packets sent in the past second
	PacketRate_Outgoing,

	// The avg. size (in bytes) of packets received
	PacketSize_Incoming,

	// The avg. size (in bytes) of packets sent
	PacketSize_Outgoing,

	// New stats should go above here
	Count UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EDisplayablePerformanceStat, EDisplayablePerformanceStat::Count);


UENUM()
enum class EFramePacingEditCondition
{
	EnableIf,
	DisableIf,
};


// How hare frame pacing and overall graphics settings controlled/exposed for the platform?
UENUM()
enum class EFramePacingMode : uint8
{
	// Manual frame rate limits, user is allowed to choose whether to lock to vsync
	DesktopStyle,

	// Limits handled by choosing present intervals driven by device profiles
	ConsoleStyle,

	// Limits handled by a user-facing choice of frame rate from among ones allowed by device profiles for the specific device
	MobileStyle
};


//////////////////////////////////////////////////////////////////////