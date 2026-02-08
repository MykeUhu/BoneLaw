#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
