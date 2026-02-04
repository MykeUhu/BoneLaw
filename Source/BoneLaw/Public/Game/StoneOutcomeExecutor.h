#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/StoneTypes.h"
#include "UObject/Object.h"
#include "StoneOutcomeExecutor.generated.h"

class UAbilitySystemComponent;
class UStoneScheduler;

struct FStoneOutcomeContext
{
	UAbilitySystemComponent* ASC = nullptr;
	FGameplayTagContainer* Tags = nullptr;
	TArray<FName>* EventPoolIds = nullptr;
	UStoneScheduler* Scheduler = nullptr;
	FStoneTimeState* Time = nullptr;
	FGameplayTag* FocusTag = nullptr;
};

UCLASS()
class BONELAW_API UStoneOutcomeExecutor : public UObject
{
	GENERATED_BODY()

public:
	void Execute(const TArray<FStoneOutcome>& Outcomes, FStoneOutcomeContext& Ctx);
};
