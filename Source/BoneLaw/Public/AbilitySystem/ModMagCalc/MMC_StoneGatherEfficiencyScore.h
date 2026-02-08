#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_StoneGatherEfficiencyScore.generated.h"

/**
 * GatherEfficiencyScore (100 = neutral baseline).
 * Survival + Hunting knowledge progression; Endurance/Strength help physically.
 */
UCLASS()
class BONELAW_API UMMC_StoneGatherEfficiencyScore : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_StoneGatherEfficiencyScore();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition EnduranceDef;
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeSurvivalDef;
	FGameplayEffectAttributeCaptureDefinition KnowledgeHuntingDef;
};
