#include "AbilitySystem/ModMagCalc/MMC_StoneInjuryResistance.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "GameplayEffectTypes.h"

namespace StoneMMC
{
	static float Primary01(float V) { return FMath::Clamp(V, 0.f, 50.f) / 50.f; }
	static float KnowledgeDelta01(float V) { return FMath::Clamp((V - 50.f) / 50.f, 0.f, 1.f); }
}

UMMC_StoneInjuryResistance::UMMC_StoneInjuryResistance()
{
	EnduranceDef.AttributeToCapture = UStoneAttributeSet::GetEnduranceAttribute();
	EnduranceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	EnduranceDef.bSnapshot = false;

	WillpowerDef.AttributeToCapture = UStoneAttributeSet::GetWillpowerAttribute();
	WillpowerDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	WillpowerDef.bSnapshot = false;

	KnowledgeMedicineDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeMedicineAttribute();
	KnowledgeMedicineDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeMedicineDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(EnduranceDef);
	RelevantAttributesToCapture.Add(WillpowerDef);
	RelevantAttributesToCapture.Add(KnowledgeMedicineDef);
}

float UMMC_StoneInjuryResistance::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float End = 0.f, Wil = 0.f, Med = 0.f;
	GetCapturedAttributeMagnitude(EnduranceDef, Spec, Params, End);
	GetCapturedAttributeMagnitude(WillpowerDef, Spec, Params, Wil);
	GetCapturedAttributeMagnitude(KnowledgeMedicineDef, Spec, Params, Med);

	End = FMath::Max(End, 0.f);
	Wil = FMath::Max(Wil, 0.f);
	Med = FMath::Max(Med, 0.f);

	// Injury resistance is mostly medical competence (progressed), with a toughness baseline.
	const float R = FMath::Clamp(
		0.35f * StoneMMC::Primary01(End) +
		0.20f * StoneMMC::Primary01(Wil) +
		0.45f * StoneMMC::KnowledgeDelta01(Med),
		0.f, 1.f);

	return FMath::Clamp(0.50f * R, 0.f, 0.50f);
}
