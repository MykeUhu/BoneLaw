#include "AbilitySystem/ModMagCalc/MMC_StoneMaxHealth.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "AbilitySystem/ModMagCalc/StoneModMagCalcHelpers.h"
#include "GameplayEffectTypes.h"

UMMC_StoneMaxHealth::UMMC_StoneMaxHealth()
{
	EnduranceDef.AttributeToCapture = UStoneAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EnduranceDef.bSnapshot = false;

	StrengthDef.AttributeToCapture = UStoneAttributeSet::GetStrengthAttribute();
	StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	StrengthDef.bSnapshot = false;

	WillpowerDef.AttributeToCapture = UStoneAttributeSet::GetWillpowerAttribute();
	WillpowerDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	WillpowerDef.bSnapshot = false;

	KnowledgeMedicineDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeMedicineAttribute();
	KnowledgeMedicineDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeMedicineDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(EnduranceDef);
	RelevantAttributesToCapture.Add(StrengthDef);
	RelevantAttributesToCapture.Add(WillpowerDef);
	RelevantAttributesToCapture.Add(KnowledgeMedicineDef);
}

float UMMC_StoneMaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Endurance = 0.f;
	float Strength = 0.f;
	float Willpower = 0.f;
	float KnowledgeMedicine = 0.f;

	GetCapturedAttributeMagnitude(EnduranceDef, Spec, Params, Endurance);
	GetCapturedAttributeMagnitude(StrengthDef, Spec, Params, Strength);
	GetCapturedAttributeMagnitude(WillpowerDef, Spec, Params, Willpower);
	GetCapturedAttributeMagnitude(KnowledgeMedicineDef, Spec, Params, KnowledgeMedicine);

	Endurance = FMath::Max(Endurance, 0.f);
	Strength = FMath::Max(Strength, 0.f);
	Willpower = FMath::Max(Willpower, 0.f);
	KnowledgeMedicine = FMath::Max(KnowledgeMedicine, 0.f);

	// --- Designer intent ---
	// Health is primarily physical robustness (Endurance), supported by Strength/Willpower.
	// Medicine increases max health only as you progress beyond the baseline knowledge (50).
	const float R =
		FMath::Clamp(
			0.45f * StoneModMagCalc::Primary01(Endurance) +
			0.20f * StoneModMagCalc::Primary01(Strength) +
			0.15f * StoneModMagCalc::Primary01(Willpower) +
			0.20f * StoneModMagCalc::KnowledgeDelta01(KnowledgeMedicine),
			0.f, 1.f);

	// Start ~100, late game up to ~1000
	const float Max = 100.f + 900.f * R;
	return FMath::Clamp(Max, 100.f, 1000.f);
}
