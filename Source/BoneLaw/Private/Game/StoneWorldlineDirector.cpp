#include "Game/StoneWorldlineDirector.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/StoneAttributeSet.h"
#include "Core/StoneGameplayTags.h"
#include "Data/StoneTypes.h"

static float ClampAxis(float V) { return FMath::Clamp(V, -100.f, 100.f); }

void UStoneWorldlineDirector::Initialize(UAbilitySystemComponent* InASC)
{
	ASC = InASC;

	Axes.Add(EStoneWorldAxis::MercyRuthless, {});
	Axes.Add(EStoneWorldAxis::TraditionInnovation, {});
	Axes.Add(EStoneWorldAxis::CollectiveIndividual, {});
	Axes.Add(EStoneWorldAxis::SpiritualPractical, {});
	Axes.Add(EStoneWorldAxis::XenoOpenXenoFear, {});
	Axes.Add(EStoneWorldAxis::TabooLooseTabooStrict, {});
}

float UStoneWorldlineDirector::GetAxisValue(EStoneWorldAxis Axis) const
{
	if (const FStoneWorldAxisState* S = Axes.Find(Axis))
	{
		return S->Value;
	}
	return 0.f;
}

// ---- Culture mapping (AAA-indie rule: derived, not duplicated) ----
// Here we map existing Culture Attributes to axis values.
// Negative = left pole, Positive = right pole.

float UStoneWorldlineDirector::ReadCultureValue_MercyRuthless() const
{
	if (!ASC) return 0.f;
	const float Empathy  = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureEmpathyAttribute());
	const float Violence = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureViolenceAttribute());
	return ClampAxis(Violence - Empathy);
}

float UStoneWorldlineDirector::ReadCultureValue_TraditionInnovation() const
{
	if (!ASC) return 0.f;
	const float Innovation = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureInnovationAttribute());
	const float Spiritual  = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureSpiritualityAttribute());
	return ClampAxis(Innovation - Spiritual);
}

float UStoneWorldlineDirector::ReadCultureValue_CollectiveIndividual() const
{
	if (!ASC) return 0.f;
	const float Collect = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureCollectivismAttribute());
	const float Hier    = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureHierarchyAttribute());
	return ClampAxis(Hier - Collect);
}

float UStoneWorldlineDirector::ReadCultureValue_SpiritualPractical() const
{
	if (!ASC) return 0.f;
	const float Spiritual  = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureSpiritualityAttribute());
	const float Innovation = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureInnovationAttribute());
	return ClampAxis(Innovation - Spiritual);
}

float UStoneWorldlineDirector::ReadCultureValue_Xeno() const
{
	if (!ASC) return 0.f;
	const float Xeno = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureXenophobiaAttribute());
	return ClampAxis((Xeno - 50.f) * 2.f);
}

float UStoneWorldlineDirector::ReadCultureValue_Taboo() const
{
	if (!ASC) return 0.f;
	const float Taboo = ASC->GetNumericAttribute(UStoneAttributeSet::GetCultureTabooStrictnessAttribute());
	return ClampAxis((Taboo - 50.f) * 2.f);
}

void UStoneWorldlineDirector::ApplyAxisTags(EStoneWorldAxis Axis, float Value, FGameplayTagContainer& RunTags)
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	// Thresholds: light stance at |>=25|, strong stance at |>=60|
	auto SetExclusive = [&](const FGameplayTag& A, const FGameplayTag& B, bool bA)
	{
		RunTags.RemoveTag(A);
		RunTags.RemoveTag(B);
		RunTags.AddTag(bA ? A : B);
	};

	switch (Axis)
	{
	case EStoneWorldAxis::MercyRuthless:
		if (FMath::Abs(Value) < 25.f) { RunTags.RemoveTag(T.Worldline_Merciful); RunTags.RemoveTag(T.Worldline_Ruthless); }
		else SetExclusive(T.Worldline_Merciful, T.Worldline_Ruthless, Value < 0.f);
		break;

	case EStoneWorldAxis::TraditionInnovation:
		if (FMath::Abs(Value) < 25.f) { RunTags.RemoveTag(T.Worldline_Tradition); RunTags.RemoveTag(T.Worldline_Innovation); }
		else SetExclusive(T.Worldline_Tradition, T.Worldline_Innovation, Value < 0.f);
		break;

	case EStoneWorldAxis::CollectiveIndividual:
		if (FMath::Abs(Value) < 25.f) { RunTags.RemoveTag(T.Worldline_Collective); RunTags.RemoveTag(T.Worldline_Individual); }
		else SetExclusive(T.Worldline_Collective, T.Worldline_Individual, Value < 0.f);
		break;

	case EStoneWorldAxis::SpiritualPractical:
		if (FMath::Abs(Value) < 25.f) { RunTags.RemoveTag(T.Worldline_Spiritual); RunTags.RemoveTag(T.Worldline_Practical); }
		else SetExclusive(T.Worldline_Spiritual, T.Worldline_Practical, Value < 0.f);
		break;

	case EStoneWorldAxis::XenoOpenXenoFear:
		if (FMath::Abs(Value) < 25.f) { RunTags.RemoveTag(T.Worldline_Xenophile); RunTags.RemoveTag(T.Worldline_Xenophobic); }
		else SetExclusive(T.Worldline_Xenophile, T.Worldline_Xenophobic, Value < 0.f);
		break;

	case EStoneWorldAxis::TabooLooseTabooStrict:
		if (FMath::Abs(Value) < 25.f) { RunTags.RemoveTag(T.Worldline_TabooLoose); RunTags.RemoveTag(T.Worldline_TabooStrict); }
		else SetExclusive(T.Worldline_TabooLoose, T.Worldline_TabooStrict, Value < 0.f);
		break;
	}
}

void UStoneWorldlineDirector::ApplyMilestones(EStoneWorldAxis Axis, float Value, FGameplayTagContainer& RunTags, FStoneTimeState& Time, TArray<FStoneScheduledEvent>& OutSchedules)
{
	const FStoneGameplayTags& Tags = FStoneGameplayTags::Get();

	FStoneWorldAxisState& AxisState = Axes.FindOrAdd(Axis);

	// Milestone levels:
	// 0 = none
	// 1 = strong (>=60)
	// 2 = extreme (>=85)
	int32 NewLevel = 0;
	if (FMath::Abs(Value) >= 85.f) NewLevel = 2;
	else if (FMath::Abs(Value) >= 60.f) NewLevel = 1;

	if (NewLevel <= AxisState.LastMilestoneLevel)
	{
		return;
	}

	// Trigger milestone once per level (escalation)
	AxisState.LastMilestoneLevel = NewLevel;

	// Helper to create a scheduled milestone event
	auto ScheduleMilestoneEvent = [&OutSchedules](const FGameplayTag& EventTag, EStoneEventPriority Priority, int32 ChoiceOffset = 0)
	{
		FStoneScheduledEvent ScheduledEvent;
		ScheduledEvent.Trigger = EStoneScheduleTrigger::AfterChoices;
		ScheduledEvent.Offset = ChoiceOffset;
		ScheduledEvent.EventTag = EventTag;
		ScheduledEvent.Priority = Priority;
		OutSchedules.Add(ScheduledEvent);
	};

	// Ruthless extreme unlocks cannibalism mechanics & schedules forced event
	if (Axis == EStoneWorldAxis::MercyRuthless && Value > 85.f)
	{
		RunTags.AddTag(Tags.Worldline_CannibalismUnlocked);
		ScheduleMilestoneEvent(Tags.MilestoneEvent_Cannibal_FirstTime, EStoneEventPriority::Forced);
	}

	// Innovation extreme unlocks toolmaker path & schedules breakthrough event
	if (Axis == EStoneWorldAxis::TraditionInnovation && Value > 85.f)
	{
		RunTags.AddTag(Tags.Worldline_ToolmakerPath);
		ScheduleMilestoneEvent(Tags.MilestoneEvent_Tools_Breakthrough, EStoneEventPriority::Forced);
	}

	// Spiritual extreme unlocks healer path & schedules ritual/healing event
	if (Axis == EStoneWorldAxis::SpiritualPractical && Value < -85.f) // spiritual side negative
	{
		RunTags.AddTag(Tags.Worldline_HealerPath);
		ScheduleMilestoneEvent(Tags.MilestoneEvent_Healer_Breakthrough, EStoneEventPriority::Forced);
	}

	// Xenophobic strong attracts raiders (reputation)
	if (Axis == EStoneWorldAxis::XenoOpenXenoFear && Value > 60.f)
	{
		RunTags.AddTag(Tags.Worldline_RaidersAttracted);
		ScheduleMilestoneEvent(Tags.MilestoneEvent_Raiders_FirstContact, EStoneEventPriority::High, 2);
	}

	// Taboo extreme unlocks special taboo-breaking events
	// Taboo extreme unlocks special taboo-breaking events
	if (Axis == EStoneWorldAxis::TabooLooseTabooStrict && FMath::Abs(Value) > 85.f)
	{
		ScheduleMilestoneEvent(Tags.MilestoneEvent_Taboo_Shattered, EStoneEventPriority::High);
	}
}

void UStoneWorldlineDirector::UpdateWorldline(FGameplayTagContainer& RunTags, FStoneTimeState& Time, TArray<FStoneScheduledEvent>& OutNewSchedules)
{
	OutNewSchedules.Reset();

	const float MercyR = ReadCultureValue_MercyRuthless();
	const float TradI  = ReadCultureValue_TraditionInnovation();
	const float CollI  = ReadCultureValue_CollectiveIndividual();
	const float SpirP  = ReadCultureValue_SpiritualPractical();
	const float Xeno   = ReadCultureValue_Xeno();
	const float Taboo  = ReadCultureValue_Taboo();

	Axes.FindOrAdd(EStoneWorldAxis::MercyRuthless).Value = MercyR;
	Axes.FindOrAdd(EStoneWorldAxis::TraditionInnovation).Value = TradI;
	Axes.FindOrAdd(EStoneWorldAxis::CollectiveIndividual).Value = CollI;
	Axes.FindOrAdd(EStoneWorldAxis::SpiritualPractical).Value = SpirP;
	Axes.FindOrAdd(EStoneWorldAxis::XenoOpenXenoFear).Value = Xeno;
	Axes.FindOrAdd(EStoneWorldAxis::TabooLooseTabooStrict).Value = Taboo;

	ApplyAxisTags(EStoneWorldAxis::MercyRuthless, MercyR, RunTags);
	ApplyAxisTags(EStoneWorldAxis::TraditionInnovation, TradI, RunTags);
	ApplyAxisTags(EStoneWorldAxis::CollectiveIndividual, CollI, RunTags);
	ApplyAxisTags(EStoneWorldAxis::SpiritualPractical, SpirP, RunTags);
	ApplyAxisTags(EStoneWorldAxis::XenoOpenXenoFear, Xeno, RunTags);
	ApplyAxisTags(EStoneWorldAxis::TabooLooseTabooStrict, Taboo, RunTags);

	ApplyMilestones(EStoneWorldAxis::MercyRuthless, MercyR, RunTags, Time, OutNewSchedules);
	ApplyMilestones(EStoneWorldAxis::TraditionInnovation, TradI, RunTags, Time, OutNewSchedules);
	ApplyMilestones(EStoneWorldAxis::SpiritualPractical, SpirP, RunTags, Time, OutNewSchedules);
	ApplyMilestones(EStoneWorldAxis::XenoOpenXenoFear, Xeno, RunTags, Time, OutNewSchedules);
}
