#include "AbilitySystem/ModMagCalc/MMC_StoneCarryCapacity.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "GameplayEffectTypes.h"

namespace StoneModMagCalc
{
	static float Primary01(float V) { return FMath::Clamp(V, 0.f, 50.f) / 50.f; }
	static float KnowledgeDelta01(float V) { return FMath::Clamp((V - 50.f) / 50.f, 0.f, 1.f); }
}

UMMC_StoneCarryCapacity::UMMC_StoneCarryCapacity()
{
	StrengthDef.AttributeToCapture = UStoneAttributeSet::GetStrengthAttribute();
	StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthDef.bSnapshot = false;

	EnduranceDef.AttributeToCapture = UStoneAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EnduranceDef.bSnapshot = false;

	KnowledgeSurvivalDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeSurvivalAttribute();
	KnowledgeSurvivalDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeSurvivalDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(StrengthDef);
	RelevantAttributesToCapture.Add(EnduranceDef);
	RelevantAttributesToCapture.Add(KnowledgeSurvivalDef);
}

float UMMC_StoneCarryCapacity::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Str = 0.f, End = 0.f, Surv = 0.f;
	GetCapturedAttributeMagnitude(StrengthDef, Spec, Params, Str);
	GetCapturedAttributeMagnitude(EnduranceDef, Spec, Params, End);
	GetCapturedAttributeMagnitude(KnowledgeSurvivalDef, Spec, Params, Surv);

	Str = FMath::Max(Str, 0.f);
	End = FMath::Max(End, 0.f);
	Surv = FMath::Max(Surv, 0.f);

	// KCD-ish: Carry is mostly Strength/Endurance. Survival only matters once you progressed beyond baseline.
	const float R = FMath::Clamp(
		0.55f * StoneModMagCalc::Primary01(Str) +
		0.25f * StoneModMagCalc::Primary01(End) +
		0.20f * StoneModMagCalc::KnowledgeDelta01(Surv),
		0.f, 1.f);

	const float Kg = 25.f + 95.f * R;
	return FMath::Clamp(Kg, 20.f, 140.f);
}
