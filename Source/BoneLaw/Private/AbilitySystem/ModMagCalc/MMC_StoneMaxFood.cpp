#include "AbilitySystem/ModMagCalc/MMC_StoneMaxFood.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "GameplayEffectTypes.h"

namespace StoneModMagCalc
{
	static float Primary01(float Value) { return FMath::Clamp(Value, 0.f, 50.f) / 50.f; }
	static float KnowledgeDelta01(float Value) { return FMath::Clamp((Value - 50.f) / 50.f, 0.f, 1.f); }
}

UMMC_StoneMaxFood::UMMC_StoneMaxFood()
{
	EnduranceDef.AttributeToCapture = UStoneAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EnduranceDef.bSnapshot = false;

	StrengthDef.AttributeToCapture = UStoneAttributeSet::GetStrengthAttribute();
	StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthDef.bSnapshot = false;

	KnowledgeSurvivalDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeSurvivalAttribute();
	KnowledgeSurvivalDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeSurvivalDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(EnduranceDef);
	RelevantAttributesToCapture.Add(StrengthDef);
	RelevantAttributesToCapture.Add(KnowledgeSurvivalDef);
}

float UMMC_StoneMaxFood::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Endurance = 0.f;
	float Strength = 0.f;
	float KnowledgeSurvival = 0.f;

	GetCapturedAttributeMagnitude(EnduranceDef, Spec, Params, Endurance);
	GetCapturedAttributeMagnitude(StrengthDef, Spec, Params, Strength);
	GetCapturedAttributeMagnitude(KnowledgeSurvivalDef, Spec, Params, KnowledgeSurvival);

	Endurance = FMath::Max(Endurance, 0.f);
	Strength = FMath::Max(Strength, 0.f);
	KnowledgeSurvival = FMath::Max(KnowledgeSurvival, 0.f);

	// Food cap = ability to endure + practical survival/rationing know-how.
	const float R =
		FMath::Clamp(
			0.30f * StoneModMagCalc::Primary01(Endurance) +
			0.15f * StoneModMagCalc::Primary01(Strength) +
			0.55f * StoneModMagCalc::KnowledgeDelta01(KnowledgeSurvival),
			0.f, 1.f);

	const float Max = 100.f + 900.f * R;
	return FMath::Clamp(Max, 100.f, 1000.f);
}
