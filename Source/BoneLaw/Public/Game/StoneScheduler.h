#pragma once

#include "CoreMinimal.h"
#include "Data/StoneTypes.h"
#include "UObject/Object.h"
#include "StoneScheduler.generated.h"

UCLASS()
class BONELAW_API UStoneScheduler : public UObject
{
	GENERATED_BODY()

public:
	void Reset(const FStoneTimeState& Time);

	void Enqueue(const FStoneScheduledEvent& Ev, const FStoneTimeState& Time);
	bool HasForcedDue(const FStoneTimeState& Time) const;
	
	void NotifyChoiceAdvanced(const FStoneTimeState& Time, int32 ChoicesAdvanced, bool bDayNightTransition, bool bIsNight);

	// Returns highest priority due event, removes it from queue. Returns false if none due.
	bool PopNextDue(FStoneScheduledEvent& Out, const FStoneTimeState& Time);

	const TArray<FStoneScheduledEvent>& GetQueue() const { return Queue; }

private:
	UPROPERTY()
	TArray<FStoneScheduledEvent> Queue;

	bool IsDue(const FStoneScheduledEvent& Ev, const FStoneTimeState& Time) const;
};
