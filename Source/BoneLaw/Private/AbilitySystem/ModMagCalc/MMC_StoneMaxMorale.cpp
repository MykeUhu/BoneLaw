#include "AbilitySystem/ModMagCalc/MMC_StoneMaxMorale.h"

#include "AbilitySystem/StoneAttributeSet.h"
#include "GameplayEffectTypes.h"

namespace StoneModMagCalc
{
	static float Primary01(float Value) { return FMath::Clamp(Value, 0.f, 50.f) / 50.f; }
	static float KnowledgeDelta01(float Value) { return FMath::Clamp((Value - 50.f) / 50.f, 0.f, 1.f); }
}

UMMC_StoneMaxMorale::UMMC_StoneMaxMorale()
{
	WillpowerDef.AttributeToCapture = UStoneAttributeSet::GetWillpowerAttribute();
	WillpowerDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	WillpowerDef.bSnapshot = false;

	SocialDef.AttributeToCapture = UStoneAttributeSet::GetSocialAttribute();
	SocialDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	SocialDef.bSnapshot = false;

	KnowledgeCourageDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeCourageAttribute();
	KnowledgeCourageDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeCourageDef.bSnapshot = false;

	KnowledgeSocialDef.AttributeToCapture = UStoneAttributeSet::GetKnowledgeSocialAttribute();
	KnowledgeSocialDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	KnowledgeSocialDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(WillpowerDef);
	RelevantAttributesToCapture.Add(SocialDef);
	RelevantAttributesToCapture.Add(KnowledgeCourageDef);
	RelevantAttributesToCapture.Add(KnowledgeSocialDef);
}

float UMMC_StoneMaxMorale::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters Params;
	Params.SourceTags = SourceTags;
	Params.TargetTags = TargetTags;

	float Willpower = 0.f;
	float Social = 0.f;
	float KnowledgeCourage = 0.f;
	float KnowledgeSocial = 0.f;

	GetCapturedAttributeMagnitude(WillpowerDef, Spec, Params, Willpower);
	GetCapturedAttributeMagnitude(SocialDef, Spec, Params, Social);
	GetCapturedAttributeMagnitude(KnowledgeCourageDef, Spec, Params, KnowledgeCourage);
	GetCapturedAttributeMagnitude(KnowledgeSocialDef, Spec, Params, KnowledgeSocial);

	Willpower = FMath::Max(Willpower, 0.f);
	Social = FMath::Max(Social, 0.f);
	KnowledgeCourage = FMath::Max(KnowledgeCourage, 0.f);
	KnowledgeSocial = FMath::Max(KnowledgeSocial, 0.f);

	// Morale cap = ability to cope (Willpower) + group stability (Social),
	// boosted by learned courage and social competence beyond baseline.
	const float R =
		FMath::Clamp(
			0.35f * StoneModMagCalc::Primary01(Willpower) +
			0.25f * StoneModMagCalc::Primary01(Social) +
			0.25f * StoneModMagCalc::KnowledgeDelta01(KnowledgeCourage) +
			0.15f * StoneModMagCalc::KnowledgeDelta01(KnowledgeSocial),
			0.f, 1.f);

	const float Max = 100.f + 900.f * R;
	return FMath::Clamp(Max, 100.f, 1000.f);
}
