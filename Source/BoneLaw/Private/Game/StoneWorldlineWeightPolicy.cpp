#include "Game/StoneWorldlineWeightPolicy.h"

#include "Core/StoneGameplayTags.h"
#include "Data/StoneEventData.h"
#include "Game/StoneWorldlineDirector.h"

void UStoneWorldlineWeightPolicy::Initialize(const UStoneWorldlineDirector* InWorldline)
{
	Worldline = InWorldline;
}

float UStoneWorldlineWeightPolicy::Axis01(float AxisValueAbs100) const
{
	const float A = FMath::Clamp(FMath::Abs(AxisValueAbs100) / 100.f, 0.f, 1.f);
	// Ease a bit: stronger drift grows faster than linear
	return FMath::Pow(A, 1.2f);
}

float UStoneWorldlineWeightPolicy::ComputeMultiplier(const UStoneEventData* Event, const FStoneSnapshot& Snapshot) const
{
	if (!Event || !Worldline) return 1.f;

	float Mul = 1.f;

	Mul = ApplyMercyRuthless(Event, Snapshot, Mul);
	Mul = ApplyTraditionInnovation(Event, Snapshot, Mul);
	Mul = ApplyXeno(Event, Snapshot, Mul);
	Mul = ApplyTaboo(Event, Snapshot, Mul);

	// Clamp to keep balancing stable
	return FMath::Clamp(Mul, 0.25f, 3.0f);
}

float UStoneWorldlineWeightPolicy::ApplyMercyRuthless(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	const float Axis = Worldline->GetAxisValue(EStoneWorldAxis::MercyRuthless); // -100 merciful .. +100 ruthless
	const float Strength = Axis01(Axis); // uses abs
	if (Strength <= 0.f) return Mul;

	// Ruthless pushes: injury/raid/violence, reduces pure-social mercy events
	if (Axis > 0.f)
	{
		if (Event->EventTags.HasTag(T.Event_Injury)) Mul *= (1.f + 0.9f * Strength);
		if (Event->EventTags.HasTag(T.Event_Social)) Mul *= (1.f - 0.35f * Strength);
	}
	// Merciful pushes: healing/social/shelter, reduces violent arcs
	else
	{
		if (Event->EventTags.HasTag(T.Event_Social)) Mul *= (1.f + 0.8f * Strength);
		if (Event->EventTags.HasTag(T.Event_Injury)) Mul *= (1.f - 0.25f * Strength);
	}

	// Milestone: cannibalism unlocked -> “dark survival” events more likely at night
	if (Snapshot.RunTags.HasTag(T.Worldline_CannibalismUnlocked) && Snapshot.Time.bIsNight)
	{
		Mul *= 1.15f;
	}

	return Mul;
}

float UStoneWorldlineWeightPolicy::ApplyTraditionInnovation(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	const float Axis = Worldline->GetAxisValue(EStoneWorldAxis::TraditionInnovation); // -100 tradition .. +100 innovation
	const float Strength = Axis01(Axis);
	if (Strength <= 0.f) return Mul;

	// Innovation pushes: craft/fire/tools/traps; Tradition pushes: spiritual/taboo/social cohesion
	if (Axis > 0.f)
	{
		if (Event->EventTags.HasTag(T.Event_Fire)) Mul *= (1.f + 0.75f * Strength);
		if (Event->EventTags.HasTag(T.Event_Shelter)) Mul *= (1.f + 0.25f * Strength);
	}
	else
	{
		if (Event->EventTags.HasTag(T.Event_Social)) Mul *= (1.f + 0.55f * Strength);
		// Night rituals (we use Event_Night tag)
		if (Event->EventTags.HasTag(T.Event_Night)) Mul *= (1.f + 0.25f * Strength);
	}

	// Milestone: toolmaker path -> craft-ish events get extra pull
	if (Snapshot.RunTags.HasTag(T.Worldline_ToolmakerPath) && Event->EventTags.HasTag(T.Event_Shelter))
	{
		Mul *= 1.10f;
	}

	return Mul;
}

float UStoneWorldlineWeightPolicy::ApplyXeno(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	const float Axis = Worldline->GetAxisValue(EStoneWorldAxis::XenoOpenXenoFear); // -100 open .. +100 fear
	const float Strength = Axis01(Axis);
	if (Strength <= 0.f) return Mul;

	// Fear pushes: night/paranoia/injury, reduces peaceful social
	if (Axis > 0.f)
	{
		if (Event->EventTags.HasTag(T.Event_Night)) Mul *= (1.f + 0.55f * Strength);
		if (Event->EventTags.HasTag(T.Event_Injury)) Mul *= (1.f + 0.35f * Strength);
		if (Event->EventTags.HasTag(T.Event_Social)) Mul *= (1.f - 0.25f * Strength);
	}
	// Open pushes: social/trade-esque (use Event_Social as proxy)
	else
	{
		if (Event->EventTags.HasTag(T.Event_Social)) Mul *= (1.f + 0.6f * Strength);
	}

	// Milestone: raiders attracted -> night danger events more frequent
	if (Snapshot.RunTags.HasTag(T.Worldline_RaidersAttracted) && Event->EventTags.HasTag(T.Event_Night))
	{
		Mul *= 1.15f;
	}

	return Mul;
}

float UStoneWorldlineWeightPolicy::ApplyTaboo(const UStoneEventData* Event, const FStoneSnapshot& Snapshot, float Mul) const
{
	const FStoneGameplayTags& T = FStoneGameplayTags::Get();

	const float Axis = Worldline->GetAxisValue(EStoneWorldAxis::TabooLooseTabooStrict); // -100 loose .. +100 strict
	const float Strength = Axis01(Axis);
	if (Strength <= 0.f) return Mul;

	// Strict: social order / punishment / ritual (proxy: Event_Social + Night)
	// Loose: experimentation / fire / forage (proxy: Event_Fire + Event_Forage)
	if (Axis > 0.f)
	{
		if (Event->EventTags.HasTag(T.Event_Social)) Mul *= (1.f + 0.5f * Strength);
		if (Event->EventTags.HasTag(T.Event_Night)) Mul *= (1.f + 0.2f * Strength);
	}
	else
	{
		if (Event->EventTags.HasTag(T.Event_Fire)) Mul *= (1.f + 0.45f * Strength);
		if (Event->EventTags.HasTag(T.Event_Forage)) Mul *= (1.f + 0.35f * Strength);
	}

	return Mul;
}
