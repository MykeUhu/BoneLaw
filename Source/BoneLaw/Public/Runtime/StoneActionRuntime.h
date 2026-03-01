#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Data/StoneEventData.h"
#include "StoneActionRuntime.generated.h"

struct FStoneChoiceResolved;
class UStoneScheduler;
class UStoneOutcomeExecutor;
class UStoneEventResolver;
class UAbilitySystemComponent;
class UStoneEventData;

UCLASS()
class BONELAW_API UStoneActionRuntime : public UObject
{
	GENERATED_BODY()

public:
	bool Init(UAbilitySystemComponent* InASC, int32 RNGSeed);

	// --- Event pick (your current SSOT) ---
	UStoneEventData* PickEventByRequiredTags(const FGameplayTagContainer& RequiredTags);
	void RebuildEventIdCache();

	// --- Choice pipeline (core mechanic) ---
	/** Resolve choices for UI (locks/hidden/softfail text, etc.) */
	void GetResolvedChoices(const UStoneEventData* Event, TArray<FStoneChoiceResolved>& OutResolved) const;

	/** Apply choice outcomes to the agent ASC (core mechanic). Returns false if choice cannot be applied. */
	bool ApplyChoice(const UStoneEventData* Event, int32 ChoiceIndex);
	
private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> ASC;

	FRandomStream RNG;

	/** Cached PrimaryAssetIds for type StoneEvent. */
	UPROPERTY()
	TArray<FPrimaryAssetId> CachedEventIds;

	bool EnsureEventIdCache();

	static FPrimaryAssetType GetStoneEventAssetType()
	{
		return FPrimaryAssetType(TEXT("StoneEvent"));
	}

	UPROPERTY()
	TObjectPtr<UStoneEventResolver> Resolver;

	UPROPERTY()
	TObjectPtr<UStoneOutcomeExecutor> OutcomeExecutor;

	UPROPERTY()
	TObjectPtr<UStoneScheduler> Scheduler;

	// If you want action-scoped runtime tags (NOT global run)
	UPROPERTY()
	FGameplayTagContainer RuntimeTags;

	UPROPERTY()
	FStoneTimeState Time;

	UPROPERTY()
	FGameplayTag FocusTag;

	void EnsureCoreSystems();

	// helper used by ApplyChoice
	void ExecuteChoiceOutcomes(const FStoneChoiceData& Choice, bool bSoftFailPath);
	void IncrementChoiceCounter();
};