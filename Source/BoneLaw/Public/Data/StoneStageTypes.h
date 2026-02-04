#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StoneTypes.h"
#include "StoneStageTypes.generated.h"

USTRUCT(BlueprintType)
struct FStoneStageState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FStoneTimeState Time;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer Tags;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Focus;

	UPROPERTY(BlueprintReadOnly)
	float Morale = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Warmth = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bFireUnlocked = false;
};
