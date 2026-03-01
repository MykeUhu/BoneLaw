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

/**
 * A single pre-rolled encounter slot on a leg (Outbound or Return).
 * TriggerAtProgress01 is a normalized position along the leg (0.0 = start, 1.0 = end).
 * This is progress-based, NOT time-based, so time manipulation (speedup/slowdown) has no effect.
 *
 * EventTag is always a PHASE tag (e.g. Action.Phase.Outbound).
 * The actual event selection happens in OpenEncounterByTag(), which builds
 * RequiredTags = {EventTag (Phase), CurrentDef->ActionTag (context)} and uses HasAll() to match.
 * This ensures e.g. a Forest Outbound slot only picks from Forest-specific Outbound events.
 */
USTRUCT(BlueprintType)
struct FStonePlannedEncounter
{
	GENERATED_BODY()

	/**
	 * Phase tag for this slot (e.g. Action.Phase.Outbound).
	 * Combined with the running action's ActionTag at trigger time to scope the event pool.
	 * Do NOT put a concrete event tag here - the random pick happens at trigger time via HasAll().
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Action")
	FGameplayTag EventTag;

	/** Normalized position along the current leg at which this encounter fires (0..1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Action", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TriggerAtProgress01 = 0.5f;

	/** Has this slot already been triggered this action? */
	bool bTriggered = false;
};
