// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "StoneAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
