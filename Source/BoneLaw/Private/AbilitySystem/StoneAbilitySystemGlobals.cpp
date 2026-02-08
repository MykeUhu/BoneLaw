// Copyright by MykeUhu


#include "AbilitySystem/StoneAbilitySystemGlobals.h"
#include "Core/StoneAbilityTypes.h"


FGameplayEffectContext* UStoneAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FStoneGameplayEffectContext();
}
