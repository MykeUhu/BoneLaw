#pragma once

#include "CoreMinimal.h"
#include "Data/StoneTypes.h"
#include "UObject/Object.h"
#include "StoneEventResolver.generated.h"

class UAbilitySystemComponent;
class UStoneEventData;

USTRUCT(BlueprintType)
struct FStoneChoiceResolved
{
	GENERATED_BODY()

	// If false, UI should not show this choice at all (e.g., hidden by requirements).
	UPROPERTY(BlueprintReadOnly, Category="Stone|Choice")
	bool bVisible = true;

	// If false, UI should show disabled state; use DisabledReason for tooltip/help text.
	UPROPERTY(BlueprintReadOnly, Category="Stone|Choice")
	bool bEnabled = true;

	// True when requirements fail but choice is still selectable and routes to FailOutcomes.
	UPROPERTY(BlueprintReadOnly, Category="Stone|Choice")
	bool bSoftFail = false;

	// Optional explanation for disabled state.
	UPROPERTY(BlueprintReadOnly, Category="Stone|Choice")
	FText DisabledReason;
};

UCLASS()
class BONELAW_API UStoneEventResolver : public UObject
{
	GENERATED_BODY()

public:
	bool EvaluateRequirement(const FStoneRequirement& Req, const UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags) const;

	void ResolveChoices(const UStoneEventData* Event, const UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags, TArray<FStoneChoiceResolved>& Out) const;

	// Weighting hook (crisis + focus + tags). Keep deterministic and cheap.
	int32 ComputeFinalWeight(const UStoneEventData* Event, const FStoneSnapshot& Snapshot) const;
};
