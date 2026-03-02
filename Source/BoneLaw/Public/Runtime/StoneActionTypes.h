#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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

/**
 * Result of an encounter/action abort check.
 * Applied by outcomes via GAS tags (Action.Abort, Action.ReturnImmediately).
 */
UENUM(BlueprintType)
enum class EStoneActionAbortResult : uint8
{
	None             UMETA(DisplayName = "None"),
	Abort            UMETA(DisplayName = "Abort (fail)"),
	ReturnImmediately UMETA(DisplayName = "Return Immediately (skip to Return phase)"),
};


USTRUCT(BlueprintType)
struct BONELAW_API FStonePlannedEncounter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerAtProgress01 = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTriggered = false;
};
