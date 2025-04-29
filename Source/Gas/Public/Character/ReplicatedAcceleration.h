#pragma once

#include "ReplicatedAcceleration.generated.h"
/**
* FReplicatedAcceleration: Compressed representation of acceleration
 */
USTRUCT()
struct FReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0; // Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0; //Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0; // Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};
