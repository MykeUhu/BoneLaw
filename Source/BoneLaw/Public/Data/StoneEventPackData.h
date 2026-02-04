#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StoneEventPackData.generated.h"

USTRUCT(BlueprintType)
struct FStonePackEntry
{
	GENERATED_BODY()

	// Studio rule: EventId == StoneEvent asset name
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName EventId;

	// Optional: weight modifier per event inside this pack
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeightMultiplier = 1.f;

	// Optional: tags to add when this event is present (rarely needed)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer EntryTags;
};

UCLASS(BlueprintType)
class BONELAW_API UStoneEventPackData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Studio rule: PackId == AssetName
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PackId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	// If these requirements are not met, pack is locked.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredTagsAll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer BlockedTagsAny;

	// If true: pack auto-adds to pool when requirements are met
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAutoUnlockWhenRequirementsMet = true;

	// If true: when unlocked, immediately preloads all events in pack
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bPreloadOnUnlock = true;

	// Packs can be “phases” (Starter / Mid / Late)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Phase = 0;

	// Main content list
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FStonePackEntry> Events;

	// Optional: pack completion criteria (e.g. at least N events seen, or tag set)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SeenCountToComplete = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer TagsToMarkCompleteAll;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		static const FPrimaryAssetType Type(TEXT("StonePack"));
		return FPrimaryAssetId(Type, GetFName());
	}
};
