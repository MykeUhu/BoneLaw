#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneInjuryResistance.generated.h"

/**
 * InjuryResistance (0..0.5).
 * Represents severity reduction (less frustrating than pure RNG chance).
 */
UCLASS()
class BONELAW_API UMMC_StoneInjuryResistance : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneInjuryResistance();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
	FGameplayEffectAttributeCaptureDefinition WillpowerDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeMedicineDef;
};
