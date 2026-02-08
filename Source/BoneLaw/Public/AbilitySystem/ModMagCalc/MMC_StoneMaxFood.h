#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneMaxFood.generated.h"

/**
 * MaxFood (Cap).
 * Surival skill progression (above baseline 50) is the main driver.
 */
UCLASS()
class BONELAW_API UMMC_StoneMaxFood : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneMaxFood();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeSurvivalDef;
};
