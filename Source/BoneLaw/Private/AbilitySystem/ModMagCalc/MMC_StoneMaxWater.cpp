#include "AbilitySystem/ModMagCalc/MMC_StoneMaxWater.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "AbilitySystem/ModMagCalc/StoneModMagCalcHelpers.h"
#include "GameplayEffectTypes.h"

UMMC_StoneMaxWater::UMMC_StoneMaxWater()
{
	EnduranceDef.AttributeToCapture = UStoneAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EnduranceDef.bSnapshot = false;

	KnowledgeSurvivalDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeSurvivalAttribute();
	KnowledgeSurvivalDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeSurvivalDef.bSnapshot = false;

	KnowledgeCraftDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeCraftAttribute();
	KnowledgeCraftDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeCraftDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(EnduranceDef);
	RelevantAttributesToCapture.Add(KnowledgeSurvivalDef);
	RelevantAttributesToCapture.Add(KnowledgeCraftDef);
}

float UMMC_StoneMaxWater::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Endurance = 0.f;
	float KnowledgeSurvival = 0.f;
	float KnowledgeCraft = 0.f;

	GetCapturedAttributeMagnitude(EnduranceDef, Spec, Params, Endurance);
	GetCapturedAttributeMagnitude(KnowledgeSurvivalDef, Spec, Params, KnowledgeSurvival);
	GetCapturedAttributeMagnitude(KnowledgeCraftDef, Spec, Params, KnowledgeCraft);

	Endurance = FMath::Max(Endurance, 0.f);
	KnowledgeSurvival = FMath::Max(KnowledgeSurvival, 0.f);
	KnowledgeCraft = FMath::Max(KnowledgeCraft, 0.f);

	// Water cap = finding/handling water (Survival) + technical mitigation (Craft) + body tolerance (Endurance).
	const float R =
		FMath::Clamp(
			0.25f * StoneModMagCalc::Primary01(Endurance) +
			0.55f * StoneModMagCalc::KnowledgeDelta01(KnowledgeSurvival) +
			0.20f * StoneModMagCalc::KnowledgeDelta01(KnowledgeCraft),
			0.f, 1.f);

	const float Max = 100.f + 900.f * R;
	return FMath::Clamp(Max, 100.f, 1000.f);
}
