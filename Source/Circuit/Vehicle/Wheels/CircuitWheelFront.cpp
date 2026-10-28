// Copyright Epic Games, Inc. All Rights Reserved.

#include "CircuitWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UCircuitWheelFront::UCircuitWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}