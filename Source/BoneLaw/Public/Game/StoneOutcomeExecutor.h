#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Data/StoneTypes.h"
#include "StoneOutcomeExecutor.generated.h"

class UAbilitySystemComponent;
class UStoneScheduler;
class UStoneRunSubsystem;

struct FStoneOutcomeContext
{
	UAbilitySystemComponent* ASC = nullptr;
	FGameplayTagContainer* Tags = nullptr;
	TArray<FName>* EventPoolIds = nullptr;
	UStoneScheduler* Scheduler = nullptr;
	FStoneTimeState* Time = nullptr;
	FGameplayTag* FocusTag = nullptr;

	// Optional: used as GameplayEffect SourceObject. If null, fall back to ASC avatar actor.
	UObject* SourceObject = nullptr;
};

UCLASS()
class BONELAW_API UStoneOutcomeExecutor : public UObject
{
	GENERATED_BODY()

public:
	void ApplyOutcome(const FStoneOutcome& O, const FStoneOutcomeContext& Ctx);
	void ApplyOutcomes(const TArray<FStoneOutcome>& Outcomes, const FStoneOutcomeContext& Ctx);
};
