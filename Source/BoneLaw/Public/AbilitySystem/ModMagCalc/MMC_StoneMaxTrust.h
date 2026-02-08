#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneMaxTrust.generated.h"

/**
 * MaxTrust (Cap).
 * Trust is primarily social competence and learned social skills (beyond baseline 50).
 * Culture intentionally not used here.
 */
UCLASS()
class BONELAW_API UMMC_StoneMaxTrust : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneMaxTrust();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition SocialDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeSocialDef;
};
