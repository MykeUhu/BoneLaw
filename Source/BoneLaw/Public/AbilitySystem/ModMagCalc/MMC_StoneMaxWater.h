#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneMaxWater.generated.h"

/**
 * MaxWater (Cap).
 * Survival progression drives it; Craft helps via containers/filtration/techniques.
 */
UCLASS()
class BONELAW_API UMMC_StoneMaxWater : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneMaxWater();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeSurvivalDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeCraftDef;
};
