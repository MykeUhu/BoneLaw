#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbility.h"
#include "StoneTypes.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class EStoneChoiceLockMode : uint8
{
	Hidden   UMETA(DisplayName="Hidden"),
	Disabled UMETA(DisplayName="Disabled"),
	SoftFail UMETA(DisplayName="SoftFail")
};

UENUM(BlueprintType)
enum class EStoneOutcomeType : uint8
{
	AttributeDelta       UMETA(DisplayName="AttributeDelta"),
	ApplyGameplayEffect  UMETA(DisplayName="ApplyGameplayEffect"),
	AddTags              UMETA(DisplayName="AddTags"),
	RemoveTags           UMETA(DisplayName="RemoveTags"),
	ForceNextEvent       UMETA(DisplayName="ForceNextEvent"),
	PoolAddEvent         UMETA(DisplayName="PoolAddEvent"),
	PoolRemoveEvent      UMETA(DisplayName="PoolRemoveEvent"),
	ScheduleEvent        UMETA(DisplayName="ScheduleEvent"),
	SetFocusTag          UMETA(DisplayName="SetFocusTag")
};

UENUM(BlueprintType)
enum class EStoneScheduleTrigger : uint8
{
	AfterChoices    UMETA(DisplayName="AfterChoices"),
	AfterDays       UMETA(DisplayName="AfterDays"),
	AfterNights     UMETA(DisplayName="AfterNights"),
	AtDayStart      UMETA(DisplayName="AtDayStart"),
	AtNightStart    UMETA(DisplayName="AtNightStart")
};

UENUM(BlueprintType)
enum class EStoneEventPriority : uint8
{
	Normal UMETA(DisplayName="Normal"),
	High   UMETA(DisplayName="High"),
	Forced UMETA(DisplayName="Forced")
};

USTRUCT(BlueprintType)
struct FStoneAttributeMin
{
	GENERATED_BODY()

	/** Attribute identified by gameplay tag (e.g. Attributes.Vital.Food). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Attributes"))
	FGameplayTag AttributeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinValue = 0.f;
};

USTRUCT(BlueprintType)
struct FStoneRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagQuery MustMatchQuery;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredTagsAll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer BlockedTagsAny;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStoneAttributeMin> MinAttributes;
};

USTRUCT(BlueprintType)
struct FStoneScheduledEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EStoneScheduleTrigger Trigger = EStoneScheduleTrigger::AfterChoices;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Offset = 1;

	/**
	 * Event identifier (preferred): gameplay tag that maps to an event in the pool.
	 * If you also set EventId, EventTag wins.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="MilestoneEvent,Event"))
	FGameplayTag EventTag;

	/** Optional direct id lookup (use only when you must reference a specific event by id). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DisplayName="Event Id (Direct)"))
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EStoneEventPriority Priority = EStoneEventPriority::Normal;

	// Internal bookkeeping (filled by scheduler)
	UPROPERTY()
	int32 DueDay = 0;

	UPROPERTY()
	int32 DueNightCount = 0;

	UPROPERTY()
	int32 DueChoiceCount = 0;

	bool IsValid() const { return EventTag.IsValid() || !EventId.IsNone(); }
};

USTRUCT(BlueprintType)
struct FStoneOutcome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EStoneOutcomeType Type = EStoneOutcomeType::AttributeDelta;

	/** For AttributeDelta: attribute tag to modify (e.g. Attributes.Vital.Food). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Attributes"))
	FGameplayTag AttributeTag;

	/** For AttributeDelta: signed delta. For other types: meaning depends on Type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Magnitude = 0.f;

	/** For ApplyGameplayEffect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	/** For AddTags/RemoveTags. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Tags;

	/** For ForceNextEvent / PoolAddEvent / PoolRemoveEvent: event id. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventId;

	/** For ScheduleEvent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStoneScheduledEvent Scheduled;
};

/**
 * Time state for a run. All time data comes from Ultra Dynamic Sky (UDS).
 * C++ does NOT calculate time internally - it only tracks counters incremented by Blueprint.
 */
USTRUCT(BlueprintType)
struct FStoneTimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 DayIndex = 1;

	UPROPERTY(BlueprintReadOnly)
	bool bIsNight = false;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalChoices = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalNightsPassed = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentHour = 6;
};

USTRUCT(BlueprintType)
struct FStoneSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FStoneTimeState Time;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer RunTags;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag FocusTag;

	UPROPERTY(BlueprintReadOnly)
	FName CurrentEventId;

	// Visible stats (cached for UI convenience; still source-of-truth is GAS)
	UPROPERTY(BlueprintReadOnly)
	float Food = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Water = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Health = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Morale = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Warmth = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Trust = 0.f;
};

/**
 * FSavedAttribute - Data-only attribute value keyed by GameplayTag
 * Used for clean DRY persistence of attributes without hardcoding names.
 */
USTRUCT(BlueprintType)
struct FSavedAttribute
{
	GENERATED_BODY()

	/** Attribute identified by gameplay tag (e.g. Attributes.Vital.Food). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Attributes"))
	FGameplayTag AttributeTag;

	/** Current value of the attribute. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;

	FSavedAttribute() = default;
	FSavedAttribute(const FGameplayTag& InTag, float InValue) : AttributeTag(InTag), Value(InValue) {}
};

/**
 * FSavedAbility - Data-only record of a granted ability (no UObject pointers stored in SaveGame).
 * NOTE: We store the ability class as TSubclassOf (serialized as class reference), plus tag metadata.
 */
USTRUCT(BlueprintType)
struct FSavedAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ClassDefaults")
	TSubclassOf<UGameplayAbility> GameplayAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults")
	FGameplayTag AbilityStatus;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults")
	FGameplayTag AbilitySlot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults")
	FGameplayTag AbilityType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Defaults")
	int32 AbilityLevel = 1;
};

inline bool operator==(const FSavedAbility& Left, const FSavedAbility& Right)
{
	return Left.AbilityTag == Right.AbilityTag;
}


/**
 * FSavedBuildable - Data-only record for runtime-built/basebuilding objects.
 *
 * IMPORTANT:
 * - Not used for placed-in-level world content. Placed actors keep their WorldObjectId in the map.
 * - Used for runtime-spawned/basebuilding actors. These records are later used to respawn objects
 *   via registry/settings lookups (no hardcoded asset paths).
 */
USTRUCT(BlueprintType)
struct FSavedBuildable
{
	GENERATED_BODY()

	/** Persistent unique identifier for this built object. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid WorldObjectId = FGuid();

	/** Type of buildable object (resolved via settings/registry). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Buildable"))
	FGameplayTag BuildableTypeTag;

	/** World transform of the placed object. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;

	/** Optional lightweight state payload (keep primitive-only; expand later). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Durability = 1.f;

	FSavedBuildable() { WorldObjectId = FGuid::NewGuid(); }
};

/**
 * FSavedAssignment - Data-only assignment state
 * NO pointers to actors or ASC - only IDs and tags.
 */
USTRUCT(BlueprintType)
struct FSavedAssignment
{
	GENERATED_BODY()

	/** Unique assignment identifier (for tracking/debugging). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid AssignmentId = FGuid();

	/** Task type tag (e.g. Task.Woodcutting, Task.Hunting). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="Task"))
	FGameplayTag TaskTag;

	/** World ID of the task actor (preferred stable reference). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TaskActorId;

	/** Fallback: task location if actor ID not available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TaskLocation = FVector::ZeroVector;

	/** When assignment started (game time in seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double StartTimeSeconds = 0.0;

	/** Expected duration of the assignment (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double DurationSeconds = 0.0;

	/** True if settler is currently "away" working (may be despawned/hidden). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAwayWorking = false;

	/** Where the settler should reappear on return. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ReturnTransform;

	/** Optional: progress tracking (elapsed seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double ElapsedSeconds = 0.0;

	FSavedAssignment() { AssignmentId = FGuid::NewGuid(); }
};

/**
 * FSavedSettler - Complete settler state for persistence
 * NO ASC/Actor pointers - only data and IDs.
 */
USTRUCT(BlueprintType)
struct FSavedSettler
{
	GENERATED_BODY()

	/** Persistent unique settler identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SettlerId = FGuid();

	/** Display name for UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/** Last known or home transform. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform LastKnownTransform;

	/** Persistent gameplay tags (traits, states, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer SettlerTags;

	/** Granted abilities (uses existing FSavedAbility). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSavedAbility> GrantedAbilities;

	/** Saved attribute values (DRY: tag -> value). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSavedAttribute> Attributes;

	/** Current assignment (if any). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSavedAssignment CurrentAssignment;

	/** True if settler has an active assignment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAssignment = false;

	FSavedSettler() { SettlerId = FGuid::NewGuid(); }
};

/**
 * FSavedOpenEventContext - Context for resuming interrupted events
 * Used when an event is open during save.
 */
USTRUCT(BlueprintType)
struct FSavedOpenEventContext
{
	GENERATED_BODY()

	/** Event tag or ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventId;

	/** Which settler is involved (if applicable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SettlerId;

	/** Associated assignment (if event occurred during assignment). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid AssignmentId;

	/** Task actor context (if applicable). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TaskActorId;

	bool IsValid() const { return EventTag.IsValid() || !EventId.IsNone(); }
};
