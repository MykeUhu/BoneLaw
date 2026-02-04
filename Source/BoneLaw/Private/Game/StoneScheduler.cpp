#include "Game/StoneScheduler.h"

void UStoneScheduler::Reset(const FStoneTimeState& Time)
{
	Queue.Reset();
}


void UStoneScheduler::Enqueue(const FStoneScheduledEvent& InEv, const FStoneTimeState& Time)
{
	FStoneScheduledEvent Ev = InEv;

	switch (Ev.Trigger)
	{
	case EStoneScheduleTrigger::AfterChoices:
		Ev.DueChoiceCount = Time.TotalChoices + Ev.Offset;
		break;
	case EStoneScheduleTrigger::AfterDays:
		Ev.DueDay = Time.DayIndex + Ev.Offset;
		break;
	case EStoneScheduleTrigger::AfterNights:
		Ev.DueNightCount = Time.TotalNightsPassed + Ev.Offset;
		break;
	case EStoneScheduleTrigger::AtDayStart:
	case EStoneScheduleTrigger::AtNightStart:
		Ev.DueChoiceCount = INT32_MAX; // will be armed on transition
		break;
	default:
		break;
	}

	Queue.Add(Ev);
}

bool UStoneScheduler::IsDue(const FStoneScheduledEvent& Ev, const FStoneTimeState& Time) const
{
	switch (Ev.Trigger)
	{
	case EStoneScheduleTrigger::AfterChoices:
		return Time.TotalChoices >= Ev.DueChoiceCount;
	case EStoneScheduleTrigger::AfterDays:
		return Time.DayIndex >= Ev.DueDay;
	case EStoneScheduleTrigger::AfterNights:
		return Time.TotalNightsPassed >= Ev.DueNightCount;
	case EStoneScheduleTrigger::AtDayStart:
	case EStoneScheduleTrigger::AtNightStart:
		return Time.TotalChoices >= Ev.DueChoiceCount;
	default:
		return false;
	}
}

bool UStoneScheduler::HasForcedDue(const FStoneTimeState& Time) const
{
	for (const auto& Ev : Queue)
	{
		if (Ev.Priority == EStoneEventPriority::Forced && IsDue(Ev, Time))
		{
			return true;
		}
	}
	return false;
}

void UStoneScheduler::NotifyChoiceAdvanced(const FStoneTimeState& Time, int32 ChoicesAdvanced, bool bDayNightTransition, bool bIsNight)
{
	// Minimal implementation: enable AtDayStart / AtNightStart triggers
	if (!bDayNightTransition)
	{
		return;
	}

	for (FStoneScheduledEvent& Ev : Queue)
	{
		if (bIsNight && Ev.Trigger == EStoneScheduleTrigger::AtNightStart)
		{
			Ev.DueChoiceCount = Time.TotalChoices; // due now
		}
		else if (!bIsNight && Ev.Trigger == EStoneScheduleTrigger::AtDayStart)
		{
			Ev.DueChoiceCount = Time.TotalChoices; // due now
		}
	}
}

bool UStoneScheduler::PopNextDue(FStoneScheduledEvent& Out, const FStoneTimeState& Time)
{
	int32 BestIdx = INDEX_NONE;
	EStoneEventPriority BestPrio = EStoneEventPriority::Normal;

	for (int32 i = 0; i < Queue.Num(); ++i)
	{
		const auto& Ev = Queue[i];
		if (!IsDue(Ev, Time)) continue;

		if (BestIdx == INDEX_NONE || static_cast<uint8>(Ev.Priority) > static_cast<uint8>(BestPrio))
		{
			BestIdx = i;
			BestPrio = Ev.Priority;
		}
	}

	if (BestIdx == INDEX_NONE)
	{
		return false;
	}

	Out = Queue[BestIdx];
	Queue.RemoveAtSwap(BestIdx);
	return true;
}
