#include "AbilitySystem/ModMagCalc/MMC_StoneMaxTrust.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "GameplayEffectTypes.h"

namespace StoneMMC
{
	static float Primary01(float Value) { return FMath::Clamp(Value, 0.f, 50.f) / 50.f; }
	static float KnowledgeDelta01(float Value) { return FMath::Clamp((Value - 50.f) / 50.f, 0.f, 1.f); }
}

UMMC_StoneMaxTrust::UMMC_StoneMaxTrust()
{
	SocialDef.AttributeToCapture = UStoneAttributeSet::GetSocialAttribute();
	SocialDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	SocialDef.bSnapshot = false;

	KnowledgeSocialDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeSocialAttribute();
	KnowledgeSocialDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeSocialDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(SocialDef);
	RelevantAttributesToCapture.Add(KnowledgeSocialDef);
}

float UMMC_StoneMaxTrust::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Social = 0.f;
	float KnowledgeSocial = 0.f;

	GetCapturedAttributeMagnitude(SocialDef, Spec, Params, Social);
	GetCapturedAttributeMagnitude(KnowledgeSocialDef, Spec, Params, KnowledgeSocial);

	Social = FMath::Max(Social, 0.f);
	KnowledgeSocial = FMath::Max(KnowledgeSocial, 0.f);

	// Trust cap = social ability + learned social competence beyond baseline.
	const float R =
		FMath::Clamp(
			0.35f * StoneMMC::Primary01(Social) +
			0.65f * StoneMMC::KnowledgeDelta01(KnowledgeSocial),
			0.f, 1.f);

	const float Max = 100.f + 900.f * R;
	return FMath::Clamp(Max, 100.f, 1000.f);
}
