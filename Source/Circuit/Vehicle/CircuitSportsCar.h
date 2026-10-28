// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CircuitPawn.h"
#include "CircuitSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class CIRCUIT_API ACircuitSportsCar : public ACircuitPawn
{
	GENERATED_BODY()
	
public:

	ACircuitSportsCar();
};
