#include "AbilitySystem/ModMagCalc/MMC_StoneTravelSpeedScore.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "AbilitySystem/ModMagCalc/StoneModMagCalcHelpers.h"
#include "GameplayEffectTypes.h"

UMMC_StoneTravelSpeedScore::UMMC_StoneTravelSpeedScore()
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

float UMMC_StoneTravelSpeedScore::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float End = 0.f, Str = 0.f, Surv = 0.f;
	GetCapturedAttributeMagnitude(EnduranceDef, Spec, Params, End);
	GetCapturedAttributeMagnitude(StrengthDef, Spec, Params, Str);
	GetCapturedAttributeMagnitude(KnowledgeSurvivalDef, Spec, Params, Surv);

	End = FMath::Max(End, 0.f);
	Str = FMath::Max(Str, 0.f);
	Surv = FMath::Max(Surv, 0.f);

	// Travel is stamina + fieldcraft. Strength helps but doesn't dominate.
	const float R = FMath::Clamp(
		0.40f * StoneModMagCalc::Primary01(End) +
		0.20f * StoneModMagCalc::Primary01(Str) +
		0.40f * StoneModMagCalc::KnowledgeDelta01(Surv),
		0.f, 1.f);

	return StoneModMagCalc::ScoreFromR(R);
}
