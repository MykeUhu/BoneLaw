#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneTravelSpeedScore.generated.h"

/**
 * TravelSpeedScore (100 = neutral baseline).
 * Used later by RuntimeSubsystem to influence action travel time.
 */
UCLASS()
class BONELAW_API UMMC_StoneTravelSpeedScore : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneTravelSpeedScore();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeSurvivalDef;
};
