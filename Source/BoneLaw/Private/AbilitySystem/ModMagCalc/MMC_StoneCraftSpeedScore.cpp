#include "AbilitySystem/ModMagCalc/MMC_StoneCraftSpeedScore.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "AbilitySystem/ModMagCalc/StoneModMagCalcHelpers.h"
#include "GameplayEffectTypes.h"

UMMC_StoneCraftSpeedScore::UMMC_StoneCraftSpeedScore()
{
	IntelligenceDef.AttributeToCapture = UStoneAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDef.bSnapshot = false;

	KnowledgeCraftDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeCraftAttribute();
	KnowledgeCraftDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeCraftDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceDef);
	RelevantAttributesToCapture.Add(KnowledgeCraftDef);
}

float UMMC_StoneCraftSpeedScore::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Int = 0.f, Craft = 0.f;
	GetCapturedAttributeMagnitude(IntelligenceDef, Spec, Params, Int);
	GetCapturedAttributeMagnitude(KnowledgeCraftDef, Spec, Params, Craft);

	Int = FMath::Max(Int, 0.f);
	Craft = FMath::Max(Craft, 0.f);

	// Craft speed should be mostly learned skill; Intelligence supports but doesn't replace practice.
	const float R = FMath::Clamp(
		0.35f * StoneModMagCalc::Primary01(Int) +
		0.65f * StoneModMagCalc::KnowledgeDelta01(Craft),
		0.f, 1.f);

	return StoneModMagCalc::ScoreFromR(R);
}
