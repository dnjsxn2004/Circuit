// Copyright Epic Games, Inc. All Rights Reserved.

#include "CircuitWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UCircuitWheelRear::UCircuitWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}