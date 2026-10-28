#pragma once

#include "CoreMinimal.h"
#include "CIRMatchTypes.generated.h"

UENUM()
enum class ECIRMatchPhase : uint8
{
	Waiting UMETA(DisplayName = "Waiting"),
	Playing UMETA(DisplayName = "Playing"),
	Finished UMETA(DisplayName = "Finished")
};