#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneCraftSpeedScore.generated.h"

/**
 * CraftSpeedScore (100 = neutral baseline).
 * Intelligence + Craft knowledge progression.
 */
UCLASS()
class BONELAW_API UMMC_StoneCraftSpeedScore : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneCraftSpeedScore();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeCraftDef;
};
