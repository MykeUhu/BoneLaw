#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttribute Attribute;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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
};

USTRUCT(BlueprintType)
struct FStoneOutcome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EStoneOutcomeType Type = EStoneOutcomeType::AttributeDelta;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Magnitude = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStoneScheduledEvent Scheduled;
};

USTRUCT(BlueprintType)
struct FStoneTimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 DayIndex = 1;

	UPROPERTY(BlueprintReadOnly)
	bool bIsNight = false;

	// Optional: step-based phase in [0..1], used for UI/sky.
	UPROPERTY(BlueprintReadOnly)
	float TimeOfDay01 = 0.25f;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalChoices = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalNightsPassed = 0;
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
