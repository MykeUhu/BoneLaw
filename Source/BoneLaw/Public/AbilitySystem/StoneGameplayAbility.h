// Copyright by MykeUhu

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "StoneGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class BONELAW_API UStoneGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;

	virtual FString GetDescription(int32 Level);
};
