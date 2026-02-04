#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneMaxHealth.generated.h"

/**
 * Calculates MaxHealth from Stone attributes (Aura-style MMC)
 * BoneLaw: No CombatInterface/Level required.
 */
UCLASS()
class BONELAW_API UMMC_StoneMaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneMaxHealth();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
};
