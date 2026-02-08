#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneMaxMorale.generated.h"

/**
 * MaxMorale (Cap).
 * Mental resilience (Willpower) + social stability (Social),
 * supported by Courage/Social knowledge progression.
 */
UCLASS()
class BONELAW_API UMMC_StoneMaxMorale : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneMaxMorale();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition WillpowerDef;
	FGameplayEffectAttributeCaptureDefinition SocialDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeCourageDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeSocialDef;
};
