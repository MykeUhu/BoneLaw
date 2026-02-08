#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneCarryCapacity.generated.h"

/**
 * Carry Capacity in kg (baseline).
 * Derived from Strength/Endurance + Survival knowledge progress (delta above 50).
 */
UCLASS()
class BONELAW_API UMMC_StoneCarryCapacity : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneCarryCapacity();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeSurvivalDef;
};
