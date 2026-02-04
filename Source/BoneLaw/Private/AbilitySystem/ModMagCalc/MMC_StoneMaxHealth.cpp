#include "AbilitySystem/ModMagCalc/MMC_StoneMaxHealth.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagsManager.h"

UMMC_StoneMaxHealth::UMMC_StoneMaxHealth()
{
	EnduranceDef.AttributeToCapture = UStoneAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EnduranceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(EnduranceDef);
}

float UMMC_StoneMaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Endurance = 0.f;
	GetCapturedAttributeMagnitude(EnduranceDef, Spec, Params, Endurance);
	Endurance = FMath::Max(Endurance, 0.f);

	// BoneLaw tuning:
	// Base 80, +2.5 per Endurance point (Aura-like curve without levels)
	const float Base = 80.f;
	const float PerEndurance = 2.5f;

	return Base + PerEndurance * Endurance;
}
