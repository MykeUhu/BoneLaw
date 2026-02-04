#pragma once

#include "CoreMinimal.h"
#include "StoneTypes.h"
#include "StoneRunTrace.generated.h"

UENUM(BlueprintType)
enum class EStoneTraceType : uint8
{
	EventPicked,
	ChoiceApplied,
	OutcomesExecuted,
	ScheduledEnqueued,
	PackUnlocked
};

USTRUCT(BlueprintType)
struct FStoneTraceEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EStoneTraceType Type = EStoneTraceType::EventPicked;

	UPROPERTY(BlueprintReadOnly)
	int32 Day = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bNight = false;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalChoices = 0;

	UPROPERTY(BlueprintReadOnly)
	FName EventId;

	UPROPERTY(BlueprintReadOnly)
	int32 ChoiceIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly)
	FString Details;

	UPROPERTY(BlueprintReadOnly)
	FDateTime RealTimeUTC;
};
