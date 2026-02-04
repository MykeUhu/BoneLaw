#include "Game/StoneWorldlineDirector.h"

#include "AbilitySystemComponent.h"
#include "Core/StoneGameplayTags.h"
#include "Data/StoneTypes.h"
#include "GAS/Attribute/StoneAttributeRegistry.h"

static float ClampAxis(float V) { return FMath::Clamp(V, -100.f, 100.f); }

void UStoneWorldlineDirector::Initialize(UAbilitySystemComponent* InASC, const UStoneAttributeRegistry* InRegistry)
{
	ASC = InASC;
	Registry = InRegistry;

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
	// Mercy <-> Ruthless:
	// Empathy high => merciful (negative), Violence high => ruthless (positive)
	if (!ASC || !Registry) return 0.f;

	FGameplayAttribute EmpathyAttr;
	FGameplayAttribute ViolenceAttr;

	// Stable names must match your registry bindings:
	Registry->TryGetByStableName("Empathy", EmpathyAttr);
	Registry->TryGetByStableName("Violence", ViolenceAttr);

	const float Empathy = EmpathyAttr.IsValid() ? ASC->GetNumericAttribute(EmpathyAttr) : 0.f;
	const float Violence = ViolenceAttr.IsValid() ? ASC->GetNumericAttribute(ViolenceAttr) : 0.f;

	// Assume attributes are in 0..100
	return ClampAxis((Violence - Empathy));
}

float UStoneWorldlineDirector::ReadCultureValue_TraditionInnovation() const
{
	if (!ASC || !Registry) return 0.f;

	FGameplayAttribute InnovationAttr;
	FGameplayAttribute SpiritualAttr;

	Registry->TryGetByStableName("Innovation", InnovationAttr);
	Registry->TryGetByStableName("Spirituality", SpiritualAttr);

	const float Innovation = InnovationAttr.IsValid() ? ASC->GetNumericAttribute(InnovationAttr) : 0.f;
	const float Spiritual = SpiritualAttr.IsValid() ? ASC->GetNumericAttribute(SpiritualAttr) : 0.f;

	// Tradition (negative) grows with spirituality/ritual; innovation (positive) with innovation
	return ClampAxis((Innovation - Spiritual));
}

float UStoneWorldlineDirector::ReadCultureValue_CollectiveIndividual() const
{
	if (!ASC || !Registry) return 0.f;

	FGameplayAttribute CollectAttr;
	FGameplayAttribute HierAttr;

	Registry->TryGetByStableName("Collectivism", CollectAttr);
	Registry->TryGetByStableName("Hierarchy", HierAttr);

	const float Collect = CollectAttr.IsValid() ? ASC->GetNumericAttribute(CollectAttr) : 0.f;
	const float Hier = HierAttr.IsValid() ? ASC->GetNumericAttribute(HierAttr) : 0.f;

	// Collective (negative) = collectivism, Individual (positive) = hierarchy/self-importance proxy
	return ClampAxis((Hier - Collect));
}

float UStoneWorldlineDirector::ReadCultureValue_SpiritualPractical() const
{
	if (!ASC || !Registry) return 0.f;

	FGameplayAttribute SpiritualAttr;
	FGameplayAttribute InnovationAttr;

	Registry->TryGetByStableName("Spirituality", SpiritualAttr);
	Registry->TryGetByStableName("Innovation", InnovationAttr);

	const float Spiritual = SpiritualAttr.IsValid() ? ASC->GetNumericAttribute(SpiritualAttr) : 0.f;
	const float Innovation = InnovationAttr.IsValid() ? ASC->GetNumericAttribute(InnovationAttr) : 0.f;

	// Spiritual (negative) vs Practical (positive) ~ innovation
	return ClampAxis((Innovation - Spiritual));
}

float UStoneWorldlineDirector::ReadCultureValue_Xeno() const
{
	if (!ASC || !Registry) return 0.f;

	FGameplayAttribute XenoAttr;
	Registry->TryGetByStableName("Xenophobia", XenoAttr);

	const float Xeno = XenoAttr.IsValid() ? ASC->GetNumericAttribute(XenoAttr) : 0.f;

	// Xenophile (negative) low xenophobia; Xenophobic (positive) high xenophobia
	return ClampAxis((Xeno - 50.f) * 2.f);
}

float UStoneWorldlineDirector::ReadCultureValue_Taboo() const
{
	if (!ASC || !Registry) return 0.f;

	FGameplayAttribute TabooAttr;
	Registry->TryGetByStableName("TabooStrictness", TabooAttr);

	const float Taboo = TabooAttr.IsValid() ? ASC->GetNumericAttribute(TabooAttr) : 0.f;

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
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	FStoneWorldAxisState& S = Axes.FindOrAdd(Axis);

	// Milestone levels:
	// 0 = none
	// 1 = strong (>=60)
	// 2 = extreme (>=85)
	int32 Level = 0;
	if (FMath::Abs(Value) >= 85.f) Level = 2;
	else if (FMath::Abs(Value) >= 60.f) Level = 1;

	if (Level <= S.LastMilestoneLevel)
	{
		return;
	}

	// Trigger milestone once per level (escalation)
	S.LastMilestoneLevel = Level;

	// Example: Ruthless extreme unlocks cannibalism mechanics & schedules a forced “hunger taboo break” event
	if (Axis == EStoneWorldAxis::MercyRuthless && Value > 85.f)
	{
		RunTags.AddTag(T.Worldline_CannibalismUnlocked);

		FStoneScheduledEvent Ev;
		Ev.Trigger = EStoneScheduleTrigger::AfterChoices;
		Ev.Offset = 0;
		Ev.EventId = FName("WL_Cannibal_FirstTime"); // create this StoneEvent asset
		Ev.Priority = EStoneEventPriority::Forced;
		OutSchedules.Add(Ev);
	}

	// Innovation extreme unlocks toolmaker path & schedules breakthrough event
	if (Axis == EStoneWorldAxis::TraditionInnovation && Value > 85.f)
	{
		RunTags.AddTag(T.Worldline_ToolmakerPath);

		FStoneScheduledEvent Ev;
		Ev.Trigger = EStoneScheduleTrigger::AfterChoices;
		Ev.Offset = 0;
		Ev.EventId = FName("WL_Breakthrough_Tools");
		Ev.Priority = EStoneEventPriority::Forced;
		OutSchedules.Add(Ev);
	}

	// Spiritual extreme unlocks healer path & schedules ritual/healing event
	if (Axis == EStoneWorldAxis::SpiritualPractical && Value < -85.f) // spiritual side negative
	{
		RunTags.AddTag(T.Worldline_HealerPath);

		FStoneScheduledEvent Ev;
		Ev.Trigger = EStoneScheduleTrigger::AfterChoices;
		Ev.Offset = 0;
		Ev.EventId = FName("WL_Ritual_Healing");
		Ev.Priority = EStoneEventPriority::Forced;
		OutSchedules.Add(Ev);
	}

	// Xenophobic strong attracts raiders (reputation)
	if (Axis == EStoneWorldAxis::XenoOpenXenoFear && Value > 60.f)
	{
		RunTags.AddTag(T.Worldline_RaidersAttracted);

		FStoneScheduledEvent Ev;
		Ev.Trigger = EStoneScheduleTrigger::AfterChoices;
		Ev.Offset = 2;
		Ev.EventId = FName("WL_Raiders_Scouts");
		Ev.Priority = EStoneEventPriority::High;
		OutSchedules.Add(Ev);
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
