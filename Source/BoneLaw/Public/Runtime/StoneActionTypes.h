#pragma once
#include "CoreMinimal.h"
#include "StoneActionTypes.generated.h"

UENUM(BlueprintType)
enum class EStoneActionPhase : uint8
{
	None,
	Outbound,
	Arrival,
	Return,
	Completed
};