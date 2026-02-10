// Copyright by MykeUhu
#include "Input/StoneInputConfig.h"

const UInputAction* UStoneInputConfig::FindActionByTag(const FGameplayTag& InputTag) const
{
	for (const FStoneInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.MatchesTagExact(InputTag))
		{
			return Action.InputAction;
		}
	}
	return nullptr;
}
